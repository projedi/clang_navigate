#pragma once

#include <unordered_map>

#include <clang/AST/ASTContext.h>
#include <clang/Frontend/Utils.h>

#include "sql.h"

namespace clang {
   class DeclRefExpr;
   class DesignatedInitExpr;
   class EnumConstantDecl;
   class EnumDecl;
   class FieldDecl;
   class FunctionDecl;
   class GotoStmt;
   class LabelDecl;
   class MemberExpr;
   class RecordDecl;
   class SourceManager;
   class SourceRange;
   class Type;
   class TypedefNameDecl;
   class VarDecl;
}

struct MyASTVisitor : clang::RecursiveASTVisitor<MyASTVisitor> {
   MyASTVisitor(clang::SourceManager const& sm, sqlite3 * db) :
      clang::RecursiveASTVisitor<MyASTVisitor>(), _sm(sm), _db(db) { }

   bool VisitTagDecl(clang::TagDecl* decl);

   bool VisitTypedefNameDecl(clang::TypedefNameDecl* decl);

   bool VisitFieldDecl(clang::FieldDecl* decl);

   bool VisitFunctionDecl(clang::FunctionDecl* decl);

   bool VisitVarDecl(clang::VarDecl* decl);

   bool VisitEnumConstantDecl(clang::EnumConstantDecl* decl);

   // TODO: What exactly is it?
   //bool VisitIndirectFieldDecl(clang::IndirectFieldDecl* decl);

   bool VisitLabelStmt(clang::LabelStmt* stmt);

   bool VisitGotoStmt(clang::GotoStmt* stmt);

   // TODO: What exactly is it?
   //bool VisitIndirectGotoStmt(clang::IndirectGotoStmt* stmt);

   bool VisitDeclRefExpr(clang::DeclRefExpr* expr);

   bool VisitMemberExpr(clang::MemberExpr* expr);

   bool VisitDesignatedInitExpr(clang::DesignatedInitExpr* expr);

   bool VisitExplicitCastExpr(clang::ExplicitCastExpr* expr);

private:
   void add_declaration(clang::SourceLocation const&, std::string const&,
         bool is_definition, clang::Decl const*);
   void add_usage(clang::SourceLocation const&, std::string const&, clang::Decl const*);
   void add_type_usage(clang::SourceLocation const&, clang::QualType const&);
   SourceRange find_range(clang::SourceLocation const&, std::string const&);
   SourceRange sql_range(clang::SourceRange const&);
   clang::Decl const* get_typedecl(clang::Type const*, std::string&);
private:
   clang::SourceManager const& _sm;
   sqlite3 * _db;
   std::unordered_map<clang::Decl const*, SourceRange> _decl_map;
};
