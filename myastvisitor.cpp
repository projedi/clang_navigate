#include <fstream>
#include <iostream>

#include <clang/AST/Decl.h>

#include "myastvisitor.h"
#include "sql.h"

// TODO: Types are not referred in casts and in function params

bool MyASTVisitor::VisitLabelDecl(clang::LabelDecl* decl) {
   add_declaration(decl->getStmt()->getIdentLoc(), decl->getStmt()->getName(), true, decl);
   return true;
}

bool MyASTVisitor::VisitTagDecl(clang::TagDecl* decl) {
   add_declaration(decl->getLocStart(), decl->getNameAsString(),
         decl->isThisDeclarationADefinition(), decl);
   return true;
}

bool MyASTVisitor::VisitTypedefNameDecl(clang::TypedefNameDecl* decl) {
   add_type_usage(decl->getTypeSourceInfo()->getTypeLoc().getLocStart(),
         decl->getUnderlyingType());
   add_declaration(decl->getLocStart(), decl->getNameAsString(), true, decl);
   return true;
}

bool MyASTVisitor::VisitFieldDecl(clang::FieldDecl* decl) {
   add_declaration(decl->getLocStart(), decl->getNameAsString(), true, decl);
   add_type_usage(decl->getLocStart(), decl->getType());
   return true;
}

// TODO: There was something about functions not iterating over defs, decls
bool MyASTVisitor::VisitFunctionDecl(clang::FunctionDecl* decl) {
   add_declaration(decl->getLocStart(), decl->getNameAsString(),
         decl->isThisDeclarationADefinition(), decl);
   // Parameters will be handled automagically by VisitVarDecl
   add_type_usage(decl->getLocStart(), decl->getResultType());
   return true;
}

bool MyASTVisitor::VisitVarDecl(clang::VarDecl* decl) {
   add_declaration(decl->getLocStart(), decl->getNameAsString(), true, decl);
   add_type_usage(decl->getLocStart(), decl->getType());
   return true;
}

bool MyASTVisitor::VisitEnumConstantDecl(clang::EnumConstantDecl* decl) {
   add_declaration(decl->getLocStart(), decl->getNameAsString(), true, decl);
   return true;
}

// TODO: What exactly is it?
//bool MyASTVisitor::VisitIndirectFieldDecl(clang::IndirectFieldDecl* decl);

bool MyASTVisitor::VisitGotoStmt(clang::GotoStmt* stmt) {
   add_usage(stmt->getLabelLoc(), stmt->getLabel()->getStmt()->getName(), stmt->getLabel());
   return true;
}

// TODO: What exactly is it?
//bool MyASTVisitor::VisitIndirectGotoStmt(clang::IndirectGotoStmt* stmt);

bool MyASTVisitor::VisitDeclRefExpr(clang::DeclRefExpr* expr) {
   add_usage(expr->getLocStart(), expr->getFoundDecl()->getNameAsString(),
         expr->getFoundDecl());
   return true;
}

bool MyASTVisitor::VisitMemberExpr(clang::MemberExpr* expr) {
   add_usage(expr->getMemberLoc(), expr->getMemberDecl()->getNameAsString(),
         expr->getMemberDecl());
   return true;
}

bool MyASTVisitor::VisitDesignatedInitExpr(clang::DesignatedInitExpr* expr) {
   for(auto it = expr->designators_begin(); it != expr->designators_end(); ++it) {
      add_usage(it->getFieldLoc(), it->getField()->getNameAsString(), it->getField());
   }
   return true;
}

void MyASTVisitor::add_declaration(clang::SourceLocation const& loc, std::string const& name,
      bool is_definition, clang::Decl const* decl) {
   ////std::cerr << "Declaration(" << is_definition << "): " << name << std::endl;
   int type = is_definition ? DEFINITION_TYPE : DECLARATION_TYPE;
   if(_decl_map.find(decl) != _decl_map.end()) {
      //std::cerr << "Found previous declaration" << std::endl;
      // TODO: Copy-paste
      SourceRange usage_range = find_range(loc, name);
      if(usage_range.filename.empty()) return;
      SourceRange decl_range = _decl_map[decl];
      int id = getDefinitionID(_db, decl_range);
      insertRow(_db, usage_range, id, name, type);
   } else {
      //std::cerr << "Haven't found previous declaration" << std::endl;
      SourceRange range = find_range(loc, name);
      if(range.filename.empty()) return;
      int id = getNewDefinitionID(_db);
      _decl_map[decl] = range;
      insertRow(_db, range, id, name, type);
   }
}

void MyASTVisitor::add_usage(clang::SourceLocation const& loc, std::string const& name,
      clang::Decl const* decl) {
   //std::cerr << "Usage: " << name << std::endl;;
   SourceRange usage_range = find_range(loc, name);
   if(usage_range.filename.empty()) return;
   SourceRange decl_range = _decl_map[decl];
   int id = getDefinitionID(_db, decl_range);
   insertRow(_db, usage_range, id, name, USAGE_TYPE);
}

void MyASTVisitor::add_type_usage(clang::SourceLocation const& loc,
      clang::QualType const& type) {
   std::string name;
   clang::Decl const* decl = get_typedecl(type.getTypePtr(), name);
   //std::cerr << "Type Usage with name " << name << std::endl;
   if(!decl) return;
   add_usage(loc, name, decl);
}

SourceRange MyASTVisitor::sql_range(clang::SourceRange const & in) {
   SourceRange out;

   clang::SourceLocation sl_begin = in.getBegin();
   clang::SourceLocation sl_end = in.getEnd();
   unsigned begin = _sm.getFileOffset(sl_begin);
   unsigned end = _sm.getFileOffset(sl_end);
   clang::FileID file = _sm.getFileID(sl_begin);

   out.filename = _sm.getFilename(sl_begin).str();
   out.row_b = _sm.getLineNumber(file, begin);
   out.col_b = _sm.getColumnNumber(file, begin);
   out.row_e = _sm.getLineNumber(file, end);
   out.col_e = _sm.getColumnNumber(file, end);

   return out;
}

SourceRange MyASTVisitor::find_range(clang::SourceLocation const& loc,
      std::string const& name) {
   SourceRange out;
   clang::FileID file = _sm.getFileID(loc);
   out.filename = _sm.getFilename(loc).str();
   unsigned begin = _sm.getFileOffset(loc);

   std::ifstream inp(out.filename);
   if(out.filename.empty()) return out;
   inp.seekg(begin);
   int64_t i, j;
   //std::cerr << "Looking for range of " << name << " in " << out.filename << ":" << begin << std::endl;
   for(i = 0, j = 0; i != name.size();++j) {
      if(inp.get() == name[i]) ++i;
      else {
         j -= i;
         inp.seekg(-i, std::ios_base::cur);
         i = 0;
      }
   }

   unsigned end = begin + j;
   begin = end - i;
   //std::cerr << "Found in between " << begin << ", " << end << std::endl;
   out.row_b = _sm.getLineNumber(file, begin);
   out.col_b = _sm.getColumnNumber(file, begin);
   out.row_e = _sm.getLineNumber(file, end);
   out.col_e = _sm.getColumnNumber(file, end);
   //std::cerr << "That is: " << out.row_b << ":" << out.col_b << " - " << out.row_e << ":"
             //<< out.col_e << std::endl;
   return out;
}

clang::Decl const* MyASTVisitor::get_typedecl(clang::Type const* type, std::string& name) {
   //static const char* names[] = {
//#define TYPE(Class, Base) #Class,
//#define ABSTRACT_TYPE(Class, Base)
//#include "clang/AST/TypeNodes.def"
   //};
   if(!type) return nullptr;
   //std::cerr << names[type->getTypeClass()] << std::endl;
   if(clang::ElaboratedType::classof(type)) {
      auto type_ = static_cast<clang::ElaboratedType const*>(type);
      return get_typedecl(type_->desugar().getTypePtr(), name);
   } else if(clang::ArrayType::classof(type)) {
      auto type_ = static_cast<clang::ArrayType const*>(type);
      return get_typedecl(type_->getElementType().getTypePtr(), name);
   }  else if(clang::TagType::classof(type)) {
      auto type_ = static_cast<clang::TagType const*>(type);
      if(!type_->getDecl()) return nullptr;
      name = type_->getDecl()->getNameAsString();
      return type_->getDecl();
   }  else if(clang::RecordType::classof(type)) {
      auto type_ = static_cast<clang::RecordType const*>(type);
      if(!type_->getDecl()) return nullptr;
      name = type_->getDecl()->getNameAsString();
      return type_->getDecl();
   } else if(clang::TypedefType::classof(type)) {
      auto type_ = static_cast<clang::TypedefType const*>(type);
      if(!type_->getDecl()) return nullptr;
      name = type_->getDecl()->getNameAsString();
      return type_->getDecl();
   } else if(clang::PointerType::classof(type)) {
      auto type_ = static_cast<clang::PointerType const*>(type);
      return get_typedecl(type_->getPointeeType().getTypePtr(), name);
   } else return nullptr;
}
