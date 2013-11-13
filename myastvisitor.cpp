#include <fstream>
#include <iostream>

#include <clang/AST/Decl.h>

#include "myastvisitor.h"
#include "sql.h"

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

bool MyASTVisitor::VisitLabelStmt(clang::LabelStmt* stmt) {
   add_declaration(stmt->getIdentLoc(), stmt->getName(), true, stmt->getDecl());
   return true;
}

bool MyASTVisitor::VisitGotoStmt(clang::GotoStmt* stmt) {
   if(!stmt->getLabel()->getStmt()) return true;
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

bool MyASTVisitor::VisitExplicitCastExpr(clang::ExplicitCastExpr* expr) {
   add_type_usage(expr->getLocStart(), expr->getTypeAsWritten());
   return true;
}

#define REDECLARABLE_OP(type, op) \
   if(auto decl_ = dynamic_cast<clang::Redeclarable<type> const*>(decl)) \
      return decl_->op();

#define IS_FIRST_DECL(type) REDECLARABLE_OP(type, isFirstDeclaration)

#define GET_FIRST_DECL(type) REDECLARABLE_OP(type, getFirstDeclaration)

bool is_first_declaration(clang::Decl const* decl) {
   IS_FIRST_DECL(clang::FunctionDecl)
   IS_FIRST_DECL(clang::TagDecl)
   IS_FIRST_DECL(clang::TypedefNameDecl)
   IS_FIRST_DECL(clang::FunctionDecl)
   IS_FIRST_DECL(clang::VarDecl)
   // Otherwise we are not redeclarable
   return true;
}

clang::Decl const* get_first_declaration(clang::Decl const* decl) {
   GET_FIRST_DECL(clang::FunctionDecl)
   GET_FIRST_DECL(clang::TagDecl)
   GET_FIRST_DECL(clang::TypedefNameDecl)
   GET_FIRST_DECL(clang::FunctionDecl)
   GET_FIRST_DECL(clang::VarDecl)
   // Otherwise we are not redeclarable
   return decl;
}

void MyASTVisitor::add_declaration(clang::SourceLocation const& loc,
      std::string const& name, bool is_definition, clang::Decl const* decl) {
   int type = is_definition ? DEFINITION_TYPE : DECLARATION_TYPE;
   if(is_first_declaration(decl)) {
      SourceRange range = find_range(loc, name);
      if(range.filename.empty()) return;
      int id = getNewDefinitionID(_db);
      _decl_map[decl] = range;
      insertRow(_db, range, id, name, type);
   } else {
      SourceRange usage_range = find_range(loc, name);
      if(usage_range.filename.empty()) return;
      SourceRange decl_range = _decl_map[get_first_declaration(decl)];
      int id = getDefinitionID(_db, decl_range);
      insertRow(_db, usage_range, id, name, type);
   }
}

void MyASTVisitor::add_usage(clang::SourceLocation const& loc, std::string const& name,
      clang::Decl const* decl) {
   if(!decl) return;
   SourceRange usage_range = find_range(loc, name);
   if(usage_range.filename.empty()) return;
   SourceRange decl_range = _decl_map[get_first_declaration(decl)];
   int id = getDefinitionID(_db, decl_range);
   if(id < 0) return;
   insertRow(_db, usage_range, id, name, USAGE_TYPE);
}

void MyASTVisitor::add_type_usage(clang::SourceLocation const& loc,
      clang::QualType const& type) {
   std::string name;
   clang::Decl const* decl = get_typedecl(type.getTypePtr(), name);
   add_usage(loc, name, decl);
}

SourceRange MyASTVisitor::find_range(clang::SourceLocation const& loc,
      std::string const& name) {
   SourceRange out;
   clang::FileID file = _sm.getFileID(loc);
   out.filename = _sm.getFilename(loc).str();
   unsigned begin = _sm.getFileOffset(loc);
   // TODO: This does not seem particularly efficient
   std::ifstream inp(out.filename);
   if(out.filename.empty()) return out;
   inp.seekg(begin);
   int64_t i, j;
   for(i = 0, j = 0; i != name.size(); ++j) {
      if(inp.get() == name[i]) ++i;
      else {
         j -= i;
         inp.seekg(-i, std::ios_base::cur);
         i = 0;
      }
   }

   unsigned end = begin + j;
   begin = end - i;
   out.row_b = _sm.getLineNumber(file, begin);
   out.col_b = _sm.getColumnNumber(file, begin);
   out.row_e = _sm.getLineNumber(file, end);
   out.col_e = _sm.getColumnNumber(file, end);
   return out;
}

clang::Decl const* MyASTVisitor::get_typedecl(clang::Type const* type, std::string& name) {
   if(!type) return nullptr;
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
