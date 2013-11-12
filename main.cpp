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
#include <llvm/Support/Host.h>
#include <llvm/Support/raw_os_ostream.h>

#include <sqlite3.h>
const int DEFINITION_TYPE = 0;
const int DECLARATION_TYPE = 1;
const int USAGE_TYPE = 2;

struct my_raw_ostream : llvm::raw_os_ostream {
   my_raw_ostream(std::ostream& ost, clang::SourceManager const& sm): llvm::raw_os_ostream(ost), _sm(sm) { }

   template<class T>
   my_raw_ostream& operator <<(T const& t) {
      static_cast<llvm::raw_ostream&>(*this) << t;
      return *this;
   }

   my_raw_ostream& operator <<(clang::SourceLocation const& loc) {
      loc.print(*this, _sm);
      return *this;
   }

   my_raw_ostream& operator <<(clang::SourceRange const& range) {
      return *this << range.getBegin() << " - " << range.getEnd();
   }

   // TODO: NamedDecl need a better way to get their bounds.
   /* DECLARATIONS */

   my_raw_ostream& operator <<(clang::LabelDecl const& decl) {
      // TODO
      return *this;
   }

   my_raw_ostream& operator <<(clang::NamespaceAliasDecl const& decl) {
      return *this << decl.getNameAsString() << "(" << decl.getAliasLoc()
                   << ") referencing " << *decl.getNamespace() // TODO: or getAliasedNamespace()?
                   << "\n   on " << decl.getTargetNameLoc();
   }

   my_raw_ostream& operator <<(clang::NamespaceDecl const& decl) {
      return *this << (decl.isAnonymousNamespace() ? "<anonymous>" : decl.getNameAsString())
                   << "(" << decl.getLocStart() << ")";
   }

   my_raw_ostream& operator <<(clang::TemplateDecl const& decl) {
      *this << decl.getTemplatedDecl()->getNameAsString() << "(" << decl.getTemplatedDecl()->getLocStart()
            << ") with parameters:";
      for(clang::NamedDecl* p: *decl.getTemplateParameters())
         *this << " " << p->getNameAsString() << "(" << p->getLocStart() << ")";
      return *this;
   }

   my_raw_ostream& operator <<(clang::EnumDecl const& decl) {
      return *this << decl.getNameAsString() << "(" << decl.getLocStart() << ")";
   }

   my_raw_ostream& operator <<(clang::RecordDecl const& decl) {
      return *this << (decl.isAnonymousStructOrUnion() ? "<anonymous>" : decl.getNameAsString())
                   << "(" << decl.getLocStart() << ")";
   }

   my_raw_ostream& operator <<(clang::CXXRecordDecl const& decl) {
      // TODO: Do I need to?(Check inheritance)
      return *this;
   }

   my_raw_ostream& operator <<(clang::ClassTemplateSpecializationDecl const& decl) {
      // TODO: Do I need to?(Check if params are generating)
      return *this;
   }

   my_raw_ostream& operator <<(clang::ClassTemplatePartialSpecializationDecl const& decl) {
      // TODO: Do I need to?(Check if params are generating)
      return *this;
   }

   my_raw_ostream& operator <<(clang::TemplateTypeParmDecl const& decl) {
      // TODO: Check if default values are handled
      return *this << decl.getNameAsString() << "(" << decl.getLocStart() << ")";
   }

   my_raw_ostream& operator <<(clang::TypedefNameDecl const& decl) {
      return *this << decl.getNameAsString() << "(" << decl.getLocStart() << ")"
                   << " referencing "; // TODO: What is it referencing?
   }

   my_raw_ostream& operator <<(clang::UsingDecl const& decl) {
      return *this << "referencing " << decl.getNameAsString() << "("
                   << decl.getQualifierLoc().getBeginLoc() << ")";
   }

   my_raw_ostream& operator <<(clang::UsingDirectiveDecl const& decl) {
      return *this << "referencing " << *decl.getNominatedNamespace();
   }

   my_raw_ostream& operator <<(clang::FieldDecl const& decl) {
      return *this << decl.getNameAsString() << "(" << decl.getQualifierLoc().getBeginLoc() << ")"
                   << " inside " << *decl.getParent() << " of type "; // TODO: Again, how to reference types?
   }

   my_raw_ostream& operator <<(clang::FunctionDecl const& decl) {
      // TODO: Return value type is handled how again?
      return *this << decl.getNameAsString() << "(" << decl.getQualifierLoc().getBeginLoc() << ")";
   }

   my_raw_ostream& operator <<(clang::CXXMethodDecl const& decl) {
      // TODO: Return value type is handled how again?
      // TODO: What about overrides?
      return *this << decl.getNameAsString() << "(" << decl.getQualifierLoc().getBeginLoc() << ")"
                   << " inside " << *decl.getParent();
   }

   my_raw_ostream& operator <<(clang::CXXConstructorDecl const& decl) {
      // TODO: Need I?
      return *this;
   }

   my_raw_ostream& operator <<(clang::CXXConversionDecl const& decl) {
      // TODO: Need I?
      return *this;
   }

   my_raw_ostream& operator <<(clang::CXXDestructorDecl const& decl) {
      // TODO: Need I?
      return *this;
   }

   my_raw_ostream& operator <<(clang::NonTypeTemplateParmDecl const& decl) {
      // TODO: It should work.
      return *this << decl.getNameAsString() << "(" << decl.getQualifierLoc().getBeginLoc() << ")"
                   << " of type "; // TODO: Types.
   }

   my_raw_ostream& operator <<(clang::VarDecl const& decl) {
      return *this << decl.getNameAsString() << "(" << decl.getQualifierLoc().getBeginLoc() << ")"
                   << " of type "; // TODO: Types.
   }

   my_raw_ostream& operator <<(clang::EnumConstantDecl const& decl) {
      return *this << decl.getNameAsString() << "(" << decl.getLocStart() << ")";
   }

   my_raw_ostream& operator <<(clang::IndirectFieldDecl const& decl) {
      // TODO: WTF is it?
      return *this;
   }

   // TODO: How try-catch, for-range are handled?
   /* STATEMENTS */

   my_raw_ostream& operator <<(clang::GotoStmt const& stmt) {
      // TODO
      return *this;
   }

   my_raw_ostream& operator <<(clang::IndirectGotoStmt const& stmt) {
      // TODO
      return *this;
   }

   /* EXPRESSIONS */

   my_raw_ostream& operator <<(clang::DeclRefExpr const& expr) {
      if(clang::VarDecl const* decl = dynamic_cast<clang::VarDecl const*>(expr.getDecl())) {
         return *this << " referencing " << *decl;
      } else {
         return *this << "ERROR: unknown DeclRefExpr at " << expr.getSourceRange();
      }
      //if(llvm::isa<clang::VarDecl const*>(expr.getDecl())) {
         //return *this << " referencing " << *llvm::dyn_cast<clang::VarDecl const*>(expr.getDecl());
      //} else {
         //return *this << "ERROR: unknown DeclRefExpr at " << expr.getSourceRange();
      //}
   }

   my_raw_ostream& operator <<(clang::DesignatedInitExpr const& expr) {
      for(auto it = expr.designators_begin(); it != expr.designators_end(); ++it) {
         *this << "\n   referencing " << *(*it).getField();
      }
      return *this;
   }

   my_raw_ostream& operator <<(clang::MemberExpr const& expr) {
      if(clang::FieldDecl const* e = dynamic_cast<clang::FieldDecl const*>(expr.getMemberDecl())) {
         return *this << *e;
      } else if(clang::CXXMethodDecl const* e = dynamic_cast<clang::CXXMethodDecl const*>(expr.getMemberDecl())) {
         return *this << *e;
      } else {
         return *this << "ERROR: unknown MemberExpr at " << expr.getSourceRange();
      }
      //if(llvm::isa<clang::FieldDecl const&>(expr)) {
         //return *this << (clang::FieldDecl const&) expr;
      //} else if(llvm::isa<clang::CXXMethodDecl const&>(expr)) {
         //return *this << (clang::CXXMethodDecl const&) expr;
      //} else {
         //return *this << "ERROR: unknown MemberExpr at " << expr.getSourceRange();
      //}
      // TODO
      //return *this;
   }

private:
   clang::SourceManager const& _sm;
};

// TODO: what about overriden operators?
struct MyASTVisitor : clang::RecursiveASTVisitor<MyASTVisitor> {
   MyASTVisitor(clang::SourceManager const& sm, sqlite3 * db) : clang::RecursiveASTVisitor<MyASTVisitor>(), llerr(std::cout, sm), _sm(sm), _db(db) { }

   bool VisitNamespaceAliasDecl(clang::NamespaceAliasDecl* decl) {
      llerr << "NamespaceAlias: " << *decl << "\n";
      return true;
   }

   bool VisitNamespaceDecl(clang::NamespaceDecl* decl) {
      llerr << "Namespace: " << *decl << "\n";
      return true;
   }

   bool VisitTypedefNameDecl(clang::TypedefNameDecl* decl) {
      llerr << "TypedefName: " << *decl << "\n";
      return true;
   }

   bool VisitEnumDecl(clang::EnumDecl* decl) {
      llerr << "Enum: " << *decl << "\n";
      return true;
   }

   bool VisitRecordDecl(clang::RecordDecl* decl) {
      llerr << "Record: " << *decl << "\n";
      return true;
   }

   bool VisitEnumConstantDecl(clang::EnumConstantDecl* decl) {
      llerr << "EnumConstant: " << *decl << "\n";
      return true;
   }

   bool VisitNonTypeTemplateParmDecl(clang::NonTypeTemplateParmDecl* decl) {
      llerr << "NonTypeTemplateParm: " << *decl << "\n";
      return true;
   }

   bool VisitFieldDecl(clang::FieldDecl* decl) {
      llerr << "Field: " << *decl << "\n";
      return true;
   }

   bool VisitFunctionDecl(clang::FunctionDecl* decl) {
      llerr << "Function: " << *decl << "\n";
      return true;
   }

   bool VisitVarDecl(clang::VarDecl* decl) {
      llerr << "Var: " << *decl << "\n";
      return true;
   }

   bool VisitMemberExpr(clang::MemberExpr* expr) {
      llerr << "Member: " << *expr << "\n";
      return true;
   }

   bool VisitDeclRefExpr(clang::DeclRefExpr* expr) {
      llerr << "DeclRef: " << *expr << "\n";
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

   void getLocation(clang::SourceRange const & range, clang::StringRef & filename,
      unsigned & row_b, unsigned & col_b,
      unsigned & row_e, unsigned & col_e)
   {
      clang::SourceLocation sl_begin = range.getBegin();
      clang::SourceLocation sl_end = range.getEnd();
      unsigned begin = _sm.getFileOffset(sl_begin);
      unsigned end = _sm.getFileOffset(sl_end);
      clang::FileID file = _sm.getFileID(sl_begin);

      filename = _sm.getFilename(sl_begin);
      row_b = _sm.getLineNumber(file, begin);
      col_b = _sm.getColumnNumber(file, begin);
      row_e = _sm.getLineNumber(file, end);
      col_e = _sm.getColumnNumber(file, end);
   }

   void insertRow(clang::SourceRange range, int id, std::string data, int type) {
      std::ostringstream query;
      sqlite3_stmt * stmt;
      int rc;

      clang::StringRef filename;
      unsigned row_b, col_b;
      unsigned row_e, col_e;

      getLocation(range, filename, row_b, col_b, row_e, col_e);

      query << "INSERT INTO items (id, file, row_b, col_b, row_e, col_e, type, data) VALUES " <<
      "(" << id << ","
          << filename.str() << ","
          << row_b << "," << col_b << ","
          << row_e << "," << col_e << ","
          << DEFINITION_TYPE << ","
          << data << ")";

      rc = sqlite3_prepare( _db, query.str().c_str(), -1, &stmt, NULL );
      if (rc != SQLITE_OK) {
         llerr << "sqlite3_prepare[" << rc << "] " << query << "\n";
         sqlite3_finalize( stmt );
         return;
      }
      rc = sqlite3_step( stmt );
      if (rc != SQLITE_DONE) {
         llerr << "sqlite3_step[" << rc << "] " << query << "\n";
         sqlite3_finalize( stmt );
         return;
      }
      sqlite3_finalize( stmt );
   }

   int getDefinitionID(clang::SourceRange range) {
      std::ostringstream query;
      sqlite3_stmt * stmt;
      int rc;

      clang::StringRef filename;
      unsigned row_b, col_b;
      unsigned row_e, col_e;

      getLocation(range, filename, row_b, col_b, row_e, col_e);

      query << "SELECT id FROM items WHERE" <<
         " filename = " << filename.str() <<
         " and row_b = " << row_b <<
         " and col_b = " << col_b <<
         " and row_e = " << row_e <<
         " and col_e = " << col_e <<
         " LIMIT 1";

      rc = sqlite3_prepare( _db, query.str().c_str(), -1, &stmt, NULL );
      if (rc != SQLITE_OK) {
         llerr << "sqlite3_prepare[" << rc << "] " << query << "\n";
         sqlite3_finalize( stmt );
         return -1;
      }
      rc = sqlite3_step( stmt );
      if (rc != SQLITE_ROW) {
         llerr << "sqlite3_step[" << rc << "] " << query << "\n";
         sqlite3_finalize( stmt );
         return -1;
      }
      int id = sqlite3_column_int( stmt, 0 );
      sqlite3_finalize( stmt );
      return id;
   }

   int getNewDefinitionID() {
      sqlite3_stmt * stmt;
      int rc;

      std::ostringstream query;
      query << "SELECT id FROM items ORDER BY id DESC LIMIT 1";

      rc = sqlite3_prepare( _db, query.str().c_str(), -1, &stmt, NULL );
      if (rc != SQLITE_OK) {
         llerr << "sqlite3_prepare[" << rc << "] " << query << "\n";
         sqlite3_finalize( stmt );
         return -1;
      }
      rc = sqlite3_step( stmt );
      if (rc != SQLITE_ROW) {
         llerr << "sqlite3_step[" << rc << "] " << query << "\n";
         sqlite3_finalize( stmt );
         return -1;
      }
      int id = sqlite3_column_int( stmt, 0 );
      sqlite3_finalize( stmt );
      return id + 1;
   }

   void createTableIfNotExists() {
      sqlite3_stmt * stmt;
      int rc;

      std::ostringstream query;
      query << "CREATE TABLE IF NOT EXISTS items (" <<
               " id INT NOT NULL," <<
               " file VARCHAR(255) NOT NULL," <<
               " row_b INT NOT NULL, col_b INT NOT NULL," <<
               " row_e INT NOT NULL, col_e INT NOT NULL," <<
               " type INT NOT NULL," <<
               " data TEXT NOT NULL );" <<
               " CREATE INDEX IF NOT EXISTS items_id_idx ON items (id);" <<
               " CREATE INDEX IF NOT EXISTS items_pos_idx ON items (file, row_b, col_b);";

      rc = sqlite3_prepare( _db, query.str().c_str(), -1, &stmt, NULL );
      if (rc != SQLITE_OK) {
         llerr << "sqlite3_prepare[" << rc << "] " << query << "\n";
         sqlite3_finalize( stmt );
         return;
      }
      rc = sqlite3_step( stmt );
      if (rc != SQLITE_DONE) {
         llerr << "sqlite3_step[" << rc << "] " << query << "\n";
         sqlite3_finalize( stmt );
         return;
      }
      sqlite3_finalize( stmt );
      return;
   }

   void dropFileIndex(std::string filename) {
      sqlite3_stmt * stmt;
      int rc;

      std::ostringstream query;
      query << "DELETE FROM items WHERE file = " << filename;

      rc = sqlite3_prepare( _db, query.str().c_str(), -1, &stmt, NULL );
      if (rc != SQLITE_OK) {
         llerr << "sqlite3_prepare[" << rc << "] " << query << "\n";
         sqlite3_finalize( stmt );
         return;
      }
      rc = sqlite3_step( stmt );
      if (rc != SQLITE_DONE) {
         llerr << "sqlite3_step[" << rc << "] " << query << "\n";
         sqlite3_finalize( stmt );
         return;
      }
      sqlite3_finalize( stmt );
      return;
   }

   void addDefitition(clang::SourceRange range, std::string data) {
      if (getDefinitionID(range) != -1) {
         return;
      }

      int id = getNewDefinitionID();
      if (id == -1) return;

      insertRow(range, id, data, DEFINITION_TYPE);
   }

   void addDeclaration(clang::SourceRange definition, clang::SourceRange range, std::string data) {
      int id = getDefinitionID(definition);
      if (id == -1) return;

      insertRow(range, id, data, DECLARATION_TYPE);
   }

   void addUsage(clang::SourceRange definition, clang::SourceRange range, std::string data) {
      int id = getDefinitionID(definition);
      if (id == -1) return;

      insertRow(range, id, data, DECLARATION_TYPE);
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

   clang::CompilerInstance ci;
   ci.createDiagnostics();
   llvm::IntrusiveRefCntPtr<clang::TargetOptions> opts(new clang::TargetOptions());
   opts->Triple = llvm::sys::getDefaultTargetTriple();
   clang::TargetInfo* info = clang::TargetInfo::CreateTargetInfo(ci.getDiagnostics(), opts.getPtr());
   ci.setTarget(info);

   ci.createFileManager();
   ci.createSourceManager(ci.getFileManager());
   ci.createPreprocessor();
   ci.getLangOpts().CPlusPlus = 1;

   //ci.getPreprocessorOpts().UsePredefines = false;
   llvm::IntrusiveRefCntPtr<clang::HeaderSearchOptions> hso( new clang::HeaderSearchOptions());
   clang::HeaderSearch headerSearch(hso, ci.getFileManager(), ci.getDiagnostics(), ci.getLangOpts(), info);

   // TODO: Platform dependent stuff. What does clang use?
   hso->AddPath("/usr/include", clang::frontend::Angled, false, false);
   hso->AddPath("/usr/include/c++/4.8.2/", clang::frontend::Angled, false, false);

   clang::InitializePreprocessor(ci.getPreprocessor(), ci.getPreprocessorOpts(), *hso, ci.getFrontendOpts());
   MyASTConsumer *astConsumer = new MyASTConsumer(db);
   ci.setASTConsumer(astConsumer);

   ci.createASTContext();

   const clang::FileEntry *pFile = ci.getFileManager().getFile(filename);
   ci.getSourceManager().createMainFileID(pFile);
   ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(),
         &ci.getPreprocessor());
   clang::ParseAST(ci.getPreprocessor(), astConsumer, ci.getASTContext());
   ci.getDiagnosticClient().EndSourceFile();

   sqlite3_close(db);

   return 0;
}
