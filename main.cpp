#include <cstdlib>
#include <iostream>
#include <fstream>

#include <clang/AST/ASTConsumer.h>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/Utils.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Tooling/Tooling.h>
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

   // TODO: NamedDecl need a better way to get their bounds.
   /* DECLARATIONS */

   my_raw_ostream& operator <<(clang::LabelDecl const& decl) {
      // TODO
      return *this;
   }

   my_raw_ostream& operator <<(clang::NamespaceAliasDecl const& decl) {
      //<< " - " << decl.getIdentifier()->getLength()
      return *this << decl.getNameAsString() << "(" << decl.getAliasLoc()
                   << ") referencing " << *decl.getNamespace()
                   << "\n   on " << decl.getTargetNameLoc();
   }

   my_raw_ostream& operator <<(clang::NamespaceDecl const& decl) {
      // TODO: Location points to '^namespace nm' instead of 'namespace ^nm'
      return *this << (decl.isAnonymousNamespace() ? "<anonymous>" : decl.getNameAsString())
                   << "(" << decl.getLocStart() << ")";
   }

   // TODO: Not needed - is handled
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
      *this << decl.getNameAsString() << "(" << decl.getLocStart()
            << ") referencing:";
      for(auto* it = decl.bases_begin(); it != decl.bases_end(); ++it) {
         *this << "\n   TODO:TYPE on " << it->getSourceRange();
      }
      return *this;
   }

   my_raw_ostream& operator <<(clang::ClassTemplateSpecializationDecl const& decl) {
      *this << decl.getNameAsString() << "(" << decl.getLocStart()
            << ") referencing:";
      // TODO: each of argument has getAsType, getAsDecl
      //for(unsigned i = 0; i != decl.getTemplateInstantiationArgs().size(); ++i)
         //*this << "\n   " << decl.getTemplateInstantiationArgs()[i];
      return *this;
   }

   // TODO: It is unneeded
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
      // TODO: I need initializer list
      return *this;
   }

   my_raw_ostream& operator <<(clang::NonTypeTemplateParmDecl const& decl) {
      // TODO: It should work.
      return *this << decl.getNameAsString() << "(" << decl.getQualifierLoc().getBeginLoc() << ")"
                   << " of type "; // TODO: Types.
   }

   my_raw_ostream& operator <<(clang::VarDecl const& decl) {
      return *this << decl.getNameAsString() << "(" << decl.getQualifierLoc().getLocalBeginLoc() << ")"
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
      } else if(clang::FunctionDecl const* decl = dynamic_cast<clang::FunctionDecl const*>(expr.getDecl())) {
         return *this << " referencing " << *decl;
      } else {
         return *this << "ERROR: unknown DeclRefExpr at " << expr.getSourceRange();
      }
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
   }

private:
   clang::SourceManager const& _sm;
};

// TODO: what about overriden operators?
struct MyASTVisitor : clang::RecursiveASTVisitor<MyASTVisitor> {
   MyASTVisitor(clang::SourceManager const& sm) : clang::RecursiveASTVisitor<MyASTVisitor>(), llerr(std::cout, sm) { }
   
   /* DECLARATIONS */

   bool VisitLabelDecl(clang::LabelDecl* decl) {
      llerr << "Label: " << *decl << "\n";
      return true;
   }

   bool VisitNamespaceAliasDecl(clang::NamespaceAliasDecl* decl) {
      llerr << "NamespaceAlias: " << *decl << "\n";
      return true;
   }

   bool VisitNamespaceDecl(clang::NamespaceDecl* decl) {
      llerr << "Namespace: " << *decl << "\n";
      return true;
   }

   bool VisitTemplateDecl(clang::TemplateDecl* decl) {
      llerr << "Template: " << *decl << "\n";
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

   bool VisitCXXRecordDecl(clang::CXXRecordDecl* decl) {
      llerr << "CXXRecord: " << *decl << "\n";
      return true;
   }

   bool VisitClassTemplateSpecializationDecl(clang::ClassTemplateSpecializationDecl* decl) {
      llerr << "ClassTemplateSpecialization: " << *decl << "\n";
      return true;
   }

   bool VisitClassTemplatePartialSpecializationDecl(clang::ClassTemplatePartialSpecializationDecl* decl) {
      llerr << "ClassTemplatePartialSpecialization: " << *decl << "\n";
      return true;
   }

   bool VisitTemplateTypeParmDecl(clang::TemplateTypeParmDecl* decl) {
      llerr << "TemplateTypeParm: " << *decl << "\n";
      return true;
   }

   bool VisitTypedefNameDecl(clang::TypedefNameDecl* decl) {
      llerr << "TypedefName: " << *decl << "\n";
      return true;
   }

   bool VisitUsingDecl(clang::UsingDecl* decl) {
      llerr << "UsingDecl: " << *decl << "\n";
      return true;
   }

   bool VisitUsingDirectiveDecl(clang::UsingDirectiveDecl* decl) {
      llerr << "UsingDirectiveDecl: " << *decl << "\n";
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

   bool VisitCXXMethodDecl(clang::CXXMethodDecl* decl) {
      llerr << "CXXMethod: " << *decl << "\n";
      return true;
   }

   bool VisitCXXConstructorDecl(clang::CXXConstructorDecl* decl) {
      llerr << "CXXConstructor: " << *decl << "\n";
      return true;
   }

   bool VisitNonTypeTemplateParmDecl(clang::NonTypeTemplateParmDecl* decl) {
      llerr << "NonTypeTemplateParm: " << *decl << "\n";
      return true;
   }

   bool VisitVarDecl(clang::VarDecl* decl) {
      llerr << "Var: " << *decl << "\n";
      return true;
   }

   bool VisitEnumConstantDecl(clang::EnumConstantDecl* decl) {
      llerr << "EnumConstant: " << *decl << "\n";
      return true;
   }

   bool VisitIndirectFieldDecl(clang::IndirectFieldDecl* decl) {
      llerr << "IndirectField: " << *decl << "\n";
      return true;
   }

   /* STATEMENTS */

   bool VisitGotoStmt(clang::GotoStmt* stmt) {
      llerr << "GotoStmt: " << *stmt << "\n";
      return true;
   }

   bool VisitIndirectGotoStmt(clang::IndirectGotoStmt* stmt) {
      llerr << "IndirectGoto: " << *stmt << "\n";
      return true;
   }

   /* EXPRESSIONS */

   bool VisitDeclRefExpr(clang::DeclRefExpr* expr) {
      llerr << "DeclRef: " << *expr << "\n";
      return true;
   }

   bool VisitDesignatedInitExpr(clang::DesignatedInitExpr* expr) {
      llerr << "DesignatedInit: " << *expr << "\n";
      return true;
   }

   bool VisitMemberExpr(clang::MemberExpr* expr) {
      llerr << "Member: " << *expr << "\n";
      return true;
   }

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

struct MyAction : clang::ASTFrontendAction {
   clang::ASTConsumer* CreateASTConsumer(clang::CompilerInstance& ci, clang::StringRef file) {
      return new MyASTConsumer();
   }
};

void usage(char* name) {
   std::cerr << "USAGE: " << name << " filename.cpp" << std::endl;
   exit(1);
}

int main(int argc, char** argv) {
   if(argc != 2) usage(argv[0]);
   std::string filename(argv[1]);

   std::vector<std::string> sources;
   sources.push_back(filename);
   clang::tooling::ClangTool tool(clang::tooling::FixedCompilationDatabase(".", std::vector<std::string>()), sources);

   return tool.run(clang::tooling::newFrontendActionFactory<MyAction>());
}
