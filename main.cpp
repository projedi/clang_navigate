#include <cstdlib>
#include <iostream>
#include <fstream>

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
private:
   clang::SourceManager const& _sm;
};

static llvm::raw_os_ostream llerr(std::cout);
static std::string llendl("\n");

struct MyASTVisitor : clang::RecursiveASTVisitor<MyASTVisitor> {
   MyASTVisitor(clang::SourceManager const& sm) : clang::RecursiveASTVisitor<MyASTVisitor>(), llerr(std::cout, sm) {
      llerr << "OHAI\n";
   }
   
   // TODO: When is it used?
   bool VisitBlockDecl(clang::BlockDecl* decl) {
      llerr << "Block @ " << decl->getSourceRange() << "\n";
      return true;
   }

   bool VisitNamespaceAliasDecl(clang::NamespaceAliasDecl* decl) {
      llerr << "NamespaceAlias @ " << decl->getSourceRange()
            << "\n   called " << decl->getNameAsString()
            << " referencing " // TODO: print what namespace is referenced
            << "\n";
      return true;
   }

   bool VisitNamespaceDecl(clang::NamespaceDecl* decl) {
      llerr << "Namespace @ " << decl->getSourceRange();
      if(decl->isAnonymousNamespace())
         llerr << "\n   anonymous";
      else
         llerr << "\n   called " << decl->getNameAsString();
      if(decl->isOriginalNamespace())
         llerr << " original\n";
      else llerr << " referencing " // TODO: print what namespace is referenced
                 << "\n";
      return true;
   }

   // TODO: Missed TemplateDecl

   bool VisitTypedefNameDecl(clang::TypedefNameDecl* decl) {
      llerr << "TypedefName @ " << decl->getSourceRange()
            << "\n   called " << decl->getNameAsString()
            << "\n"; // TODO: add type information
      return true;
   }

   bool VisitEnumDecl(clang::EnumDecl* decl) {
      llerr << "Enum @ " << decl->getSourceRange()
            << "\n   called " << decl->getNameAsString()
            << "\n"; // TODO: add type information
      return true;
   }

   bool VisitRecordDecl(clang::RecordDecl* decl) {
      llerr << "Record @ " << decl->getSourceRange()
            << "\n   called " << decl->getNameAsString()
            << "\n"; // TODO: add type information
      return true;
   }

   // TODO: WTF is IndirectFieldDecl

   bool VisitEnumConstantDecl(clang::EnumConstantDecl* decl) {
      llerr << "EnumConstant @ " << decl->getSourceRange()
            << "\n   called " << decl->getNameAsString()
            << "\n"; // TODO: add type information
      return true;
   }

   bool VisitNonTypeTemplateParmDecl(clang::NonTypeTemplateParmDecl* decl) {
      // TODO: Handle it
      llerr << "NonTypeTemplateParm\n";
      return true;
   }

   bool VisitFieldDecl(clang::FieldDecl* decl) {
      llerr << "Field @ " << decl->getSourceRange()
            << "\n   called " << decl->getNameAsString()
            << "\n"; // TODO: add type information
      return true;
   }

   // TODO: There's also CXXMethodDecl but we don't do C++ just yet
   bool VisitFunctionDecl(clang::FunctionDecl* decl) {
      llerr << "Function @ " << decl->getSourceRange()
            << "\n   called " << decl->getNameAsString()
            << "\n"; // TODO: add type information and parameter info?
      return true;
   }

   bool VisitVarDecl(clang::VarDecl* decl) {
      llerr << "Var @ " << decl->getSourceRange()
            << "\n   called " << decl->getNameAsString()
            << "\n"; // TODO: add type information and parameter info?
      return true;
   }

   // TODO: How will it play with VarDecl(badly)?
   bool VisitParmVarDecl(clang::ParmVarDecl* decl) {
      llerr << "ParmVar @ " << decl->getSourceRange()
            << "\n   called " << decl->getNameAsString()
            << "\n"; // TODO: add type information and parameter info?
      return true;
   }

   //TODO: Missed VarTemplateSpecializationDecl

   bool VisitMemberExpr(clang::MemberExpr* expr) {
      llerr << "ParmVar @ " << expr->getSourceRange()
            << "\n   referencing " << expr->getMemberDecl()->getNameAsString()
            << "\n";
      return true;
   }

   bool VisitDeclRefExpr(clang::DeclRefExpr* expr) {
      llerr << "DeclRef @ " << expr->getSourceRange()
            << "\n   referencing " << expr->getDecl()->getNameAsString()
            << "\n";
      return true;
   }
   // TODO: Should I do call, cast?
private:
   my_raw_ostream llerr;
};

struct MyASTConsumer : clang::ASTConsumer {
   MyASTConsumer() : clang::ASTConsumer() { }

   void HandleTranslationUnit(clang::ASTContext& ctx) {
      MyASTVisitor visitor(ctx.getSourceManager());
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

   clang::CompilerInstance ci;
   ci.createDiagnostics();
   llvm::IntrusiveRefCntPtr<clang::TargetOptions> opts(new clang::TargetOptions());
   opts->Triple = llvm::sys::getDefaultTargetTriple();
   clang::TargetInfo* info = clang::TargetInfo::CreateTargetInfo(ci.getDiagnostics(), opts.getPtr());
   ci.setTarget(info);

   ci.createFileManager();
   ci.createSourceManager(ci.getFileManager());
   ci.createPreprocessor();

   //ci.getPreprocessorOpts().UsePredefines = false;
   llvm::IntrusiveRefCntPtr<clang::HeaderSearchOptions> hso( new clang::HeaderSearchOptions());
   clang::HeaderSearch headerSearch(hso, ci.getFileManager(), ci.getDiagnostics(), ci.getLangOpts(), info);

   // TODO: Platform dependent stuff. What does clang use?
   hso->AddPath("/usr/include", clang::frontend::Angled, false, false);
   hso->AddPath("/usr/include/c++/4.8.2/", clang::frontend::Angled, false, false);

   clang::InitializePreprocessor(ci.getPreprocessor(), ci.getPreprocessorOpts(), *hso, ci.getFrontendOpts());
   MyASTConsumer *astConsumer = new MyASTConsumer();
   ci.setASTConsumer(astConsumer);

   ci.createASTContext();

   const clang::FileEntry *pFile = ci.getFileManager().getFile(filename);
   ci.getSourceManager().createMainFileID(pFile);
   ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(),
         &ci.getPreprocessor());
   clang::ParseAST(ci.getPreprocessor(), astConsumer, ci.getASTContext());
   ci.getDiagnosticClient().EndSourceFile();
   return 0;
}
