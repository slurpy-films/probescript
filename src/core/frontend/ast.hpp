#pragma once
#include "frontend/lexer.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <memory>

enum NodeType {
    Program,
    ProbeDeclaration,
    NumericLiteral,
    StringLiteral,
    Identifier,
    BinaryExpr,
    FunctionDeclaration,
    NullLiteral,
    VarDeclaration,
    AssignmentExpr,
    PropertyLiteral,
    MapLiteral,
    ArrayLiteral,
    MemberExpr,
    MemberAssignment,
    CallExpr,
    IfStmt,
    UndefinedLiteral,
    ImportStmt,
    ExportStmt,
    WhileStmt,
    ClassDefinition,
    NewExpr,
    ReturnStmt,
    ForStmt,
    UnaryPostFix,
    UnaryPrefix,
    ArrowFunction,
    BreakStmt,
    ContinueStmt,
    ThrowStmt,
    TryStmt,
    BoolLiteral,
    TernaryExpr,
    TemplateArgument,
    TemplateCall,
    CastExpr,
};

struct Stmt {
    Lexer::Token token = Lexer::Token();
    NodeType kind;
    Stmt(NodeType kind) : kind(kind) {}
    virtual ~Stmt() = default;
    virtual std::string toString() const { return ""; }
    virtual std::string value() const { return "default"; }
};

struct ProgramType : public Stmt {
    ProgramType() : Stmt(NodeType::Program) {}
    ProgramType(std::vector<Stmt*> body) : Stmt(NodeType::Program), body(body) {}
    std::vector<Stmt*> body;
    
    ~ProgramType() {
        for (auto stmt : body) {
            delete stmt;
        }
    }
};

struct ReturnStmtType : public Stmt {
    Stmt* stmt;
    ReturnStmtType(Stmt* stmt)
        : Stmt(NodeType::ReturnStmt), stmt(stmt) {}
    
};

struct Expr : public Stmt { 
    Expr(NodeType kind = NodeType::NullLiteral) : Stmt(kind) {}

    std::string toString() const override {
        return value();
    }
};

struct VarDeclarationType : public Stmt {
    VarDeclarationType(Expr* value, std::string ident, bool constant = false)
        : Stmt(NodeType::VarDeclaration),
          value(value),
          identifier(std::move(ident)),
          constant(constant),
          staticType(false),
          type(nullptr) {}

    VarDeclarationType(Expr* value, std::string ident, Expr* type)
        : Stmt(NodeType::VarDeclaration),
          value(value),
          identifier(std::move(ident)),
          constant(false),
          staticType(true),
          type(type) {}

    Expr* value;
    std::string identifier;
    bool constant;
    bool staticType;
    Expr* type;
};

struct UndefinedLiteralType : public Expr {
    UndefinedLiteralType() : Expr(NodeType::UndefinedLiteral) {}
    std::string value() const override {
        return "undefined";
    };
};

struct FunctionDeclarationType : public Stmt {
    FunctionDeclarationType(std::vector<VarDeclarationType*> params, std::string name, std::vector<Stmt*> body) : Stmt(NodeType::FunctionDeclaration), parameters(params), name(name), body(body) {}
    FunctionDeclarationType(std::vector<VarDeclarationType*> params, std::string name, std::vector<Stmt*> body, Expr* rettype) : Stmt(NodeType::FunctionDeclaration), parameters(params), name(name), body(body), rettype(rettype), staticRet(true) {}

    bool staticRet = false;
    Expr* rettype = new UndefinedLiteralType();
    std::vector<VarDeclarationType*> parameters;
    std::vector<VarDeclarationType*> templateparams;
    std::string name;
    std::vector<Stmt*> body;
};

struct ExportStmtType : public Stmt {
    ExportStmtType(Stmt* value) : Stmt(NodeType::ExportStmt), exporting(value) {}
    Stmt* exporting;
};

struct ThrowStmtType : public Stmt {
    ThrowStmtType(Expr* err) : Stmt(NodeType::ThrowStmt), err(err) {}
    Expr* err;
};

struct ImportStmtType : public Stmt {
    ImportStmtType(std::string name, Expr* module) : Stmt(NodeType::ImportStmt), module(module), name(name), hasMember(true) {}
    ImportStmtType(std::string name) : Stmt(NodeType::ImportStmt), name(name) {}
    ImportStmtType(std::string name, Expr* module, std::string ident) : Stmt(NodeType::ImportStmt), module(module), name(name), hasMember(true), ident(ident), customIdent(true) {}
    ImportStmtType(std::string name, std::string ident) : Stmt(NodeType::ImportStmt), name(name), ident(ident), customIdent(true) {}
    Expr* module;
    bool hasMember = false;
    bool customIdent = false;
    std::string name;
    std::string ident;
};

struct WhileStmtType : public Stmt {
    WhileStmtType(Expr* condition, std::vector<Stmt*> body)
        : Stmt(NodeType::WhileStmt), condition(condition), body(body) {}
        Expr* condition;
        std::vector<Stmt*> body;
    };

struct ProbeDeclarationType : public Stmt {
    ProbeDeclarationType(std::string name, std::vector<Stmt*> body) : Stmt(NodeType::ProbeDeclaration), name(name), body(body) {}
    ProbeDeclarationType(std::string name, std::vector<Stmt*> body, Expr* extends) : Stmt(NodeType::ProbeDeclaration), name(name), body(body), extends(extends), doesExtend(true) {}

    bool doesExtend = false;
    std::string name;
    Expr* extends;
    std::vector<Stmt*> body;
};


struct ForStmtType : public Stmt {
    std::vector<Stmt*> declarations;
    std::vector<Expr*> conditions;
    std::vector<Expr*> updates;
    std::vector<Stmt*> body;

    ForStmtType(std::vector<Stmt*> decl, std::vector<Expr*> cond, std::vector<Expr*> update, std::vector<Stmt*> body)
        : declarations(decl), conditions(cond), updates(update), body(body), Stmt(NodeType::ForStmt) {}
};

struct IfStmtType : public Stmt {
    IfStmtType(Expr* condition, std::vector<Stmt*> body) : Stmt(NodeType::IfStmt), condition(condition), body(body) {}

    Expr* condition;
    std::vector<Stmt*> body;
    std::vector<Stmt*> elseStmt;
    bool hasElse = false;
};

struct TryStmtType : public Stmt {
    std::vector<Stmt*> body;
    FunctionDeclarationType* catchHandler;

    TryStmtType(std::vector<Stmt*> body, FunctionDeclarationType* catchHandler) : Stmt(NodeType::TryStmt), body(body), catchHandler(catchHandler) {}
};

struct AssignmentExprType : public Expr {
    AssignmentExprType(Expr* assigne, Expr* value, std::string op) : Expr(NodeType::AssignmentExpr), assigne(assigne), value(value), op(op) {}

    Expr* assigne;
    Expr* value;
    std::string op;
};

struct TemplateArgumentType : public Expr {
    std::vector<Expr*> arguments;
    
    TemplateArgumentType(std::vector<Expr*> args) 
        : arguments(args) {
        kind = NodeType::TemplateArgument;
    }
};

struct CastExprType : public Expr
{
    Expr* left;
    Expr* type;

    CastExprType(Expr* left, Expr* type)
        : left(left), type(type), Expr(NodeType::CastExpr) {}
};

struct TemplateCallType : public Expr {
    Expr* caller;
    std::vector<Expr*> templateArgs;
    
    TemplateCallType(Expr* caller, std::vector<Expr*> args) 
        : caller(caller), templateArgs(args) {
        kind = NodeType::TemplateCall;
    }
};

struct TernaryExprType : public Expr {
    Expr* cond;
    Expr* cons;
    Expr* alt;

    TernaryExprType(Expr* cond, Expr* cons, Expr* alt)
        : Expr(NodeType::TernaryExpr), cond(cond), cons(cons), alt(alt) {}
};

struct UnaryPostFixType : public Expr {
    std::string op;
    Expr* assigne;

    UnaryPostFixType(std::string op, Expr* assigne)
        : op(op), assigne(assigne), Expr(NodeType::UnaryPostFix) {}
};

struct UnaryPrefixType : public Expr {
    std::string op;
    Expr* assigne;

    UnaryPrefixType(std::string op, Expr* assigne)
        : op(op), assigne(assigne), Expr(NodeType::UnaryPrefix) {}
};

struct BinaryExprType : public Expr {
    BinaryExprType(Expr* left, Expr* right, const std::string& op) 
        : Expr(NodeType::BinaryExpr), left(left), right(right), op(op) {}
    
    ~BinaryExprType() {
        delete left;
        delete right;
    }

    Expr* left;
    Expr* right;
    std::string op;
    
    std::string value() const override {
        return left->toString() + " " + op + " " + right->toString();
    }
};

struct IdentifierType : public Expr {
    IdentifierType(const std::string& symbol) 
        : Expr(NodeType::Identifier), symbol(symbol) {}

    std::string symbol;
    
    std::string value() const override {
        return symbol;
    }
};

struct NumericLiteralType : public Expr {
    NumericLiteralType(double val) 
        : Expr(NodeType::NumericLiteral), numValue(val) {}
    
    double numValue;
    
    std::string value() const override {
        return std::to_string(numValue);
    }
};

struct StringLiteralType : public Expr {
    std::string strValue;
    StringLiteralType(std::string val) : Expr(NodeType::StringLiteral), strValue(val) {}

    std::string value() const override {
        return strValue;
    }
};

struct NullLiteralType : public Expr {
    NullLiteralType() : Expr(NodeType::NullLiteral) {}
    std::string value() const override {
        return "null";
    };
};

struct BoolLiteralType : public Expr {
    bool value;
    BoolLiteralType(bool value) : Expr(NodeType::BoolLiteral), value(value) {}
};

struct ClassDefinitionType : public Stmt {
    ClassDefinitionType(std::string name, std::vector<Stmt*> body) : Stmt(NodeType::ClassDefinition), name(name), body(body) {}
    ClassDefinitionType(std::string name, std::vector<Stmt*> body, Expr* extends) : Stmt(NodeType::ClassDefinition), name(name), body(body), extends(extends), doesExtend(true) {}
    std::string name;
    std::vector<Stmt*> body;
    Expr* extends;
    bool doesExtend = false;
};

struct PropertyLiteralType : public Expr {
    PropertyLiteralType(std::string key, Expr* val) : Expr(NodeType::PropertyLiteral), key(key), val(val) {}
    std::string value() const override {
        return "null";
    };

    std::string key;
    Expr* val;
};

struct MapLiteralType : public Expr {
    MapLiteralType(std::vector<PropertyLiteralType*> properties) : Expr(NodeType::MapLiteral), properties(properties) {}
    std::string value() const override {
        return "null";
    };

    std::vector<PropertyLiteralType*> properties;
};

struct ArrayLiteralType : public Expr {
    ArrayLiteralType(std::vector<Expr*> items) : Expr(NodeType::ArrayLiteral), items(items) {}

    std::vector<Expr*> items;
};

struct ArrowFunctionType : public Expr {
    std::vector<VarDeclarationType*> params;
    std::vector<Stmt*> body;

    ArrowFunctionType(std::vector<VarDeclarationType*> params, std::vector<Stmt*> body) :
        Expr(NodeType::ArrowFunction), params(params), body(body) {}
};

struct NewExprType : public Expr {
    Expr* constructor;
    std::vector<Expr*> args;

    NewExprType(Expr* constructor, std::vector<Expr*> args) : Expr(NodeType::NewExpr), constructor(constructor), args(args) {}
};

struct CallExprType : public Expr {
    CallExprType(Expr* calee = new Expr(), std::vector<Expr*> args = {}) : Expr(NodeType::CallExpr), args(args), calee(calee) {}
    std::vector<Expr*> args;
    Expr* calee;
};

struct MemberExprType : public Expr {
    MemberExprType(Expr* object = new Expr(), Expr* property = new Expr(), bool computed = false) : Expr(NodeType::MemberExpr), object(object), property(property), computed(computed) {}
    MemberExprType(Expr* object, Expr* property, bool computed, std::string lastProp) : Expr(NodeType::MemberExpr), object(object), property(property), computed(computed), lastProp(lastProp) {}
    Expr* object;
    Expr* property;
    std::string lastProp;
    bool computed;
};

struct MemberAssignmentType : public Expr {
    MemberAssignmentType(Expr* obj, Expr* property, Expr* value, bool computed) :
        Expr(NodeType::MemberAssignment), object(obj), newvalue(value),property(property), computed(computed) {}
    Expr* object;
    Expr* property;
    Expr* newvalue;
    bool computed;
};


struct BreakStmtType : public Stmt {
    BreakStmtType() : Stmt(NodeType::BreakStmt) {}
};

struct ContinueStmtType : public Stmt {
    ContinueStmtType() : Stmt(NodeType::ContinueStmt) {}
};