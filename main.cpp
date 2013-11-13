#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/Utils.h>
#include "clang/Lex/HeaderSearch.h"
#include <clang/Parse/ParseAST.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_os_ostream.h>

#include "sql.h"

// TODO: what about overriden operators?
struct MyASTVisitor : clang::RecursiveASTVisitor<MyASTVisitor> {
   MyASTVisitor(clang::SourceManager const& sm, sqlite3 * db) :
      clang::RecursiveASTVisitor<MyASTVisitor>(), _sm(sm), _db(db) { }

   bool VisitNamespaceAliasDecl(clang::NamespaceAliasDecl* decl) {
      return true;
   }

   bool VisitNamespaceDecl(clang::NamespaceDecl* decl) {
      return true;
   }

   bool VisitTypedefNameDecl(clang::TypedefNameDecl* decl) {
      // TODO: ranges
      addUsage(decl->getSourceRange(), "Type " + decl->getUnderlyingType().getAsString(),
            findTypeLocation(decl->getUnderlyingType()->getTypePtr()),
            "Type " + decl->getUnderlyingType().getAsString());
      addDefinition(
            decl->getSourceRange(),
            "Typedef " + decl->getNameAsString() );
      _typedecl.push_back(std::make_pair(decl->getTypeForDecl(),
               decl->getSourceRange()));
      return true;
   }

   bool VisitEnumDecl(clang::EnumDecl* decl) {
      // TODO: c++11 typed enums
      clang::EnumDecl * def = decl->getDefinition();
      if (decl == def) {
         addDefinition(
            decl->getSourceRange(),
            "Enum " + decl->getNameAsString() );
      } else {
         addDeclaration(
            decl->getSourceRange(),
            "Enum " + decl->getNameAsString(),
            def->getSourceRange(),
            "Enum " + def->getNameAsString());
      }
      return true;
   }

   bool VisitRecordDecl(clang::RecordDecl* decl) {
      clang::RecordDecl * def = decl->getDefinition();
      if (decl == def) {
         addDefinition(
            decl->getSourceRange(),
            "Record " + decl->getNameAsString() );
         _typedecl.push_back(std::make_pair(decl->getTypeForDecl(),
                  decl->getSourceRange()));
      } else {
         addDeclaration(
            decl->getSourceRange(),
            "Record " + decl->getNameAsString(),
            def->getSourceRange(),
            "Record " + def->getNameAsString());
      }
      return true;
   }

   bool VisitEnumConstantDecl(clang::EnumConstantDecl* decl) {
      return true;
   }

   bool VisitNonTypeTemplateParmDecl(clang::NonTypeTemplateParmDecl* decl) {
      return true;
   }

   bool VisitFieldDecl(clang::FieldDecl* decl) {
      addDefinition(
         decl->getSourceRange(),
         "Field " + decl->getNameAsString() );
      addUsage(decl->getTypeInfo()->getTypeLoc()->getSourceRange(),
            "Type " + decl->getType().getAsString(),
            findTypeLocation(decl->getType()->getTypePtr()),
            "Type " + decl->getType().getAsString());
      return true;
   }

   bool VisitFunctionDecl(clang::FunctionDecl* decl) {
      llerr << "Function: " << *decl << "\n";
      addDefinition(
         decl->getSourceRange(),
         "Function " + decl->getNameAsString() );
      return true;
   }

   bool VisitVarDecl(clang::VarDecl* decl) {
      llerr << "Var: " << *decl << "\n";
      clang::VarDecl * def = decl->getDefinition();
      if (decl == def) {
         addDefinition(
            decl->getSourceRange(),
            "Var " + decl->getNameAsString() );
      } else {
         addDeclaration(
            decl->getSourceRange(),
            "Var " + decl->getNameAsString(),
            def->getSourceRange(),
            "Var " + def->getNameAsString());
      }
      return true;
   }

   bool VisitMemberExpr(clang::MemberExpr* expr) {
      llerr << "Member: " << *expr << "\n";
      if(clang::VarDecl const* decl = dynamic_cast<clang::VarDecl const*>(expr->getMemberDecl())) {
         addUsage(
            expr->getSourceRange(),
            "Var " + expr->getMemberNameInfo().getAsString(),
            decl->getSourceRange(),
            "Var " + decl->getNameAsString());
      } else if(clang::FieldDecl const* decl = dynamic_cast<clang::FieldDecl const*>(expr->getMemberDecl())) {
         addUsage(
            expr->getSourceRange(),
            "Field " + expr->getMemberNameInfo().getAsString(),
            decl->getSourceRange(),
            "Field " + decl->getNameAsString());
      } else if(clang::FunctionDecl const* decl = dynamic_cast<clang::FunctionDecl const*>(expr->getMemberDecl())) {
         addUsage(
            expr->getSourceRange(),
            "Fun " + expr->getMemberNameInfo().getAsString(),
            decl->getSourceRange(),
            "Fun " + decl->getNameAsString());
      }
      return true;
   }

   bool VisitDeclRefExpr(clang::DeclRefExpr* expr) {
      llerr << "DeclRef: " << *expr << "\n";
      if(clang::VarDecl const* decl = dynamic_cast<clang::VarDecl const*>(expr->getDecl())) {
         addUsage(
            expr->getSourceRange(),
            "Var " + expr->getNameInfo().getAsString(),
            decl->getSourceRange(),
            "Var " + decl->getNameAsString());
      } else if(clang::FieldDecl const* decl = dynamic_cast<clang::FieldDecl const*>(expr->getDecl())) {
         addUsage(
            expr->getSourceRange(),
            "Field " + expr->getNameInfo().getAsString(),
            decl->getSourceRange(),
            "Field " + decl->getNameAsString());
      } else if(clang::FunctionDecl const* decl = dynamic_cast<clang::FunctionDecl const*>(expr->getDecl())) {
         addUsage(
            expr->getSourceRange(),
            "Fun " + expr->getNameInfo().getAsString(),
            decl->getSourceRange(),
            "Fun " + decl->getNameAsString());
      }
      return true;
   }

   bool VisitDesignatedInitExpr(clang::DesignatedInitExpr* expr) {
      llerr << "DesignatedInit: " << *expr << "\n";
      return true;
   }

private:
   my_raw_ostream llerr;
   clang::SourceManager const& _sm;
   sqlite3 * _db;
   std::vector<std::pair<clang::Type*, clang::SourceRange>> _typedecls;

   SourceRange getLocation(clang::SourceRange const & in) {
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

   int addDefinition(clang::SourceRange range, std::string data) {
      SourceRange range_ = getLocation (range);

      int id = getDefinitionID(_db, range_);
      if (id != -1) {
         return id;
      }

      id = getNewDefinitionID(_db);
      if (id == -1) return -1;

      insertRow(_db, range_, id, data, DEFINITION_TYPE);
      return id;
   }

   void addDeclaration(clang::SourceRange range, std::string data, clang::SourceRange definition, std::string def_data) {
      SourceRange definition_ = getLocation (definition);
      SourceRange range_ = getLocation (range);

      int id = addDefinition(definition, def_data);
      if (id == -1) return;

      insertRow(_db, range_, id, data, DECLARATION_TYPE);
   }

   void addUsage(clang::SourceRange range, std::string data, clang::SourceRange definition, std::string def_data) {
      SourceRange definition_ = getLocation (definition);
      SourceRange range_ = getLocation (range);

      int id = addDefinition(definition, def_data);
      if (id == -1) return;

      insertRow(_db, range_, id, data, USAGE_TYPE);
   }
};

struct MyASTConsumer : clang::ASTConsumer {
   sqlite3 * _db;

   MyASTConsumer(sqlite3 * db) : clang::ASTConsumer(), _db(db) { }

   void HandleTranslationUnit(clang::ASTContext& ctx) {
      MyASTVisitor visitor(ctx.getSourceManager(), _db);
      visitor.TraverseDecl(ctx.getTranslationUnitDecl());
   }
};

struct MyAction : clang::ASTFrontendAction {
   MyAction(sqlite3* db): clang::ASTFrontendAction(), _db(db) { }

   clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance&, clang::StringRef) {
      return new MyASTConsumer(_db);
   }
private:
   sqlite3* _db;
};

struct MyActionFactory : clang::tooling::FrontendActionFactory {
   MyActionFactory(sqlite3* db): clang::tooling::FrontendActionFactory(), _db(db) { }

   clang::FrontendAction* create() { return new MyAction(_db); }
private:
   sqlite3* _db;
};

void usage(char* name) {
   std::cerr << "USAGE: " << name << " filename.cpp" << std::endl;
   exit(1);
}

int main(int argc, char** argv) {
   if(argc != 2) usage(argv[0]);
   std::string filename(argv[1]);

   sqlite3 *db;
   int rc = sqlite3_open("vimide.db", &db);
   if (rc != SQLITE_OK) {
      std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
      exit(1);
   }
   createTableIfNotExists(db);
   dropFileIndex(db, filename);

   std::vector<std::string> sources;
   sources.push_back(filename);
   clang::tooling::ClangTool tool(clang::tooling::FixedCompilationDatabase(".",
            std::vector<std::string>()), sources);

   int res = tool.run(new MyActionFactory(db));

   sqlite3_close(db);

   return res;
}
