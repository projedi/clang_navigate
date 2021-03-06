#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include "clang/Lex/HeaderSearch.h"
#include <clang/Parse/ParseAST.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_os_ostream.h>

#include <sqlite3.h>

#include "sql.h"
#include "myastvisitor.h"

struct MyASTConsumer : clang::ASTConsumer {
   MyASTConsumer(sqlite3 * db) : clang::ASTConsumer(), _db(db) { }

   void HandleTranslationUnit(clang::ASTContext& ctx) {
      MyASTVisitor visitor(ctx.getSourceManager(), _db);
      visitor.TraverseDecl(ctx.getTranslationUnitDecl());
   }
private:
   sqlite3* _db;
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

void usage(char const* name) {
   std::cerr << "USAGE: " << name << " filename.cpp" << std::endl;
   exit(1);
}

int main(int argc, char** argv) {
   if(argc != 2) usage(argv[0]);
   std::string filename(argv[1]);

   const size_t last_slash_idx = filename.find_last_of("\\/");
   if (std::string::npos == last_slash_idx) {
      std::cerr << "Can't get working directory" << std::endl;
      exit(1);
   }
   std::stringstream ss;
   ss << filename.substr(0, last_slash_idx) << "/vimide.db";

   sqlite3 *db;
   if (sqlite3_open(ss.str().c_str(), &db) != SQLITE_OK) {
      std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
      exit(1);
   }

   createTableIfNotExists(db);
   // TODO: But we also need to drop tables for all encountered headers
   //dropFileIndex(db, filename);
   dropBase(db);

   // TODO: Add multiple files
   std::vector<std::string> sources;
   sources.push_back(filename);
   clang::tooling::ClangTool tool(clang::tooling::FixedCompilationDatabase(".",
            std::vector<std::string>()), sources);

   int res = tool.run(new MyActionFactory(db));

   sqlite3_close(db);

   return res;
}
