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
    AwaitExpr,
};

struct Expr;

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
    ProgramType(std::vector<std::shared_ptr<Stmt>> body) : Stmt(NodeType::Program), body(body) {}
    std::vector<std::shared_ptr<Stmt>> body;
};

struct ReturnStmtType : public Stmt {
    std::shared_ptr<Expr> val;
    ReturnStmtType(std::shared_ptr<Expr> val)
        : Stmt(NodeType::ReturnStmt), val(val) {}
};

struct Expr : public Stmt { 
    Expr(NodeType kind = NodeType::NullLiteral) : Stmt(kind) {}

    std::string toString() const override {
        return value();
    }
};

struct VarDeclarationType : public Stmt {
    VarDeclarationType(std::shared_ptr<Expr> value, std::string ident, bool constant = false)
        : Stmt(NodeType::VarDeclaration),
          value(value),
          identifier(std::move(ident)),
          constant(constant),
          staticType(false),
          type(nullptr) {}

    VarDeclarationType(std::shared_ptr<Expr> value, std::string ident, std::shared_ptr<Expr> type)
        : Stmt(NodeType::VarDeclaration),
          value(value),
          identifier(std::move(ident)),
          constant(false),
          staticType(true),
          type(type) {}

    std::shared_ptr<Expr> value;
    std::string identifier;
    bool constant;
    bool staticType;
    std::shared_ptr<Expr> type;
};

struct UndefinedLiteralType : public Expr {
    UndefinedLiteralType() : Expr(NodeType::UndefinedLiteral) {}
    std::string value() const override {
        return "undefined";
    };
};

struct FunctionDeclarationType : public Stmt {
    FunctionDeclarationType(std::vector<std::shared_ptr<VarDeclarationType>> params, std::string name, std::vector<std::shared_ptr<Stmt>> body, bool isAsync = false) : Stmt(NodeType::FunctionDeclaration), parameters(params), name(name), body(body), isAsync(isAsync) {}
    FunctionDeclarationType(std::vector<std::shared_ptr<VarDeclarationType>> params, std::string name, std::vector<std::shared_ptr<Stmt>> body, std::shared_ptr<Expr> rettype, bool isAsync = false) : Stmt(NodeType::FunctionDeclaration), parameters(params), name(name), body(body), rettype(rettype), staticRet(true), isAsync(isAsync) {}

    bool staticRet = false;
    std::shared_ptr<Expr> rettype = std::make_shared<UndefinedLiteralType>();
    std::vector<std::shared_ptr<VarDeclarationType>> parameters;
    std::vector<std::shared_ptr<VarDeclarationType>> templateparams = {};
    std::string name;
    std::vector<std::shared_ptr<Stmt>> body;
    bool isAsync = false;
};

struct ExportStmtType : public Stmt {
    ExportStmtType(std::shared_ptr<Stmt> value) : Stmt(NodeType::ExportStmt), exporting(value) {}
    std::shared_ptr<Stmt> exporting;
};

struct ThrowStmtType : public Stmt {
    ThrowStmtType(std::shared_ptr<Expr> err) : Stmt(NodeType::ThrowStmt), err(err) {}
    std::shared_ptr<Expr> err;
};

struct ImportStmtType : public Stmt {
    ImportStmtType(std::string name, std::shared_ptr<Expr> module) : Stmt(NodeType::ImportStmt), module(module), name(name), hasMember(true) {}
    ImportStmtType(std::string name) : Stmt(NodeType::ImportStmt), name(name) {}
    ImportStmtType(std::string name, std::shared_ptr<Expr> module, std::string ident) : Stmt(NodeType::ImportStmt), module(module), name(name), hasMember(true), ident(ident), customIdent(true) {}
    ImportStmtType(std::string name, std::string ident) : Stmt(NodeType::ImportStmt), name(name), ident(ident), customIdent(true) {}
    std::shared_ptr<Expr> module;
    bool hasMember = false;
    bool customIdent = false;
    std::string name;
    std::string ident;
};

struct WhileStmtType : public Stmt {
    WhileStmtType(std::shared_ptr<Expr> condition, std::vector<std::shared_ptr<Stmt>> body)
        : Stmt(NodeType::WhileStmt), condition(condition), body(body) {}
        std::shared_ptr<Expr> condition;
        std::vector<std::shared_ptr<Stmt>> body;
    };

struct ProbeDeclarationType : public Stmt {
    ProbeDeclarationType(std::string name, std::vector<std::shared_ptr<Stmt>> body) : Stmt(NodeType::ProbeDeclaration), name(name), body(body) {}
    ProbeDeclarationType(std::string name, std::vector<std::shared_ptr<Stmt>> body, std::shared_ptr<Expr> extends) : Stmt(NodeType::ProbeDeclaration), name(name), body(body), extends(extends), doesExtend(true) {}

    bool doesExtend = false;
    std::string name;
    std::shared_ptr<Expr> extends;
    std::vector<std::shared_ptr<Stmt>> body;
};


struct ForStmtType : public Stmt {
    std::vector<std::shared_ptr<Stmt>> declarations;
    std::vector<std::shared_ptr<Expr>> conditions;
    std::vector<std::shared_ptr<Expr>> updates;
    std::vector<std::shared_ptr<Stmt>> body;

    ForStmtType(std::vector<std::shared_ptr<Stmt>> decl, std::vector<std::shared_ptr<Expr>> cond, std::vector<std::shared_ptr<Expr>> update, std::vector<std::shared_ptr<Stmt>> body)
        : declarations(decl), conditions(cond), updates(update), body(body), Stmt(NodeType::ForStmt) {}
};

struct IfStmtType : public Stmt {
    IfStmtType(std::shared_ptr<Expr> condition, std::vector<std::shared_ptr<Stmt>> body) : Stmt(NodeType::IfStmt), condition(condition), body(body) {}

    std::shared_ptr<Expr> condition;
    std::vector<std::shared_ptr<Stmt>> body;
    std::vector<std::shared_ptr<Stmt>> elseStmt;
    bool hasElse = false;
};

struct TryStmtType : public Stmt {
    std::vector<std::shared_ptr<Stmt>> body;
    std::shared_ptr<FunctionDeclarationType> catchHandler;

    TryStmtType(std::vector<std::shared_ptr<Stmt>> body, std::shared_ptr<FunctionDeclarationType> catchHandler) : Stmt(NodeType::TryStmt), body(body), catchHandler(catchHandler) {}
};

struct AssignmentExprType : public Expr {
    AssignmentExprType(std::shared_ptr<Expr> assigne, std::shared_ptr<Expr> value, std::string op) : Expr(NodeType::AssignmentExpr), assigne(assigne), value(value), op(op) {}

    std::shared_ptr<Expr> assigne;
    std::shared_ptr<Expr> value;
    std::string op;
};

struct TemplateArgumentType : public Expr {
    std::vector<std::shared_ptr<Expr>> arguments;
    
    TemplateArgumentType(std::vector<std::shared_ptr<Expr>> args) 
        : arguments(args) {
        kind = NodeType::TemplateArgument;
    }
};

struct CastExprType : public Expr
{
    std::shared_ptr<Expr> left;
    std::shared_ptr<Expr> type;

    CastExprType(std::shared_ptr<Expr> left, std::shared_ptr<Expr> type)
        : left(left), type(type), Expr(NodeType::CastExpr) {}
};

struct TemplateCallType : public Expr {
    std::shared_ptr<Expr> caller;
    std::vector<std::shared_ptr<Expr>> templateArgs;
    
    TemplateCallType(std::shared_ptr<Expr> caller, std::vector<std::shared_ptr<Expr>> args) 
        : caller(caller), templateArgs(args) {
        kind = NodeType::TemplateCall;
    }
};

struct TernaryExprType : public Expr {
    std::shared_ptr<Expr> cond;
    std::shared_ptr<Expr> cons;
    std::shared_ptr<Expr> alt;

    TernaryExprType(std::shared_ptr<Expr> cond, std::shared_ptr<Expr> cons, std::shared_ptr<Expr> alt)
        : Expr(NodeType::TernaryExpr), cond(cond), cons(cons), alt(alt) {}
};

struct UnaryPostFixType : public Expr {
    std::string op;
    std::shared_ptr<Expr> assigne;

    UnaryPostFixType(std::string op, std::shared_ptr<Expr> assigne)
        : op(op), assigne(assigne), Expr(NodeType::UnaryPostFix) {}
};

struct UnaryPrefixType : public Expr {
    std::string op;
    std::shared_ptr<Expr> assigne;

    UnaryPrefixType(std::string op, std::shared_ptr<Expr> assigne)
        : op(op), assigne(assigne), Expr(NodeType::UnaryPrefix) {}
};

struct BinaryExprType : public Expr {
    BinaryExprType(std::shared_ptr<Expr> left, std::shared_ptr<Expr> right, const std::string& op) 
        : Expr(NodeType::BinaryExpr), left(left), right(right), op(op) {}

    std::shared_ptr<Expr> left;
    std::shared_ptr<Expr> right;
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
    ClassDefinitionType(std::string name, std::vector<std::shared_ptr<Stmt>> body) : Stmt(NodeType::ClassDefinition), name(name), body(body) {}
    ClassDefinitionType(std::string name, std::vector<std::shared_ptr<Stmt>> body, std::shared_ptr<Expr> extends) : Stmt(NodeType::ClassDefinition), name(name), body(body), extends(extends), doesExtend(true) {}
    std::string name;
    std::vector<std::shared_ptr<Stmt>> body;
    std::shared_ptr<Expr> extends;
    bool doesExtend = false;
};

struct PropertyLiteralType : public Expr {
    PropertyLiteralType(std::string key, std::shared_ptr<Expr> val) : Expr(NodeType::PropertyLiteral), key(key), val(val) {}
    std::string value() const override {
        return "null";
    };

    std::string key;
    std::shared_ptr<Expr> val;
};

struct MapLiteralType : public Expr {
    MapLiteralType(std::vector<std::shared_ptr<PropertyLiteralType>> properties) : Expr(NodeType::MapLiteral), properties(properties) {}
    std::string value() const override {
        return "null";
    };

    std::vector<std::shared_ptr<PropertyLiteralType>> properties;
};

struct ArrayLiteralType : public Expr {
    ArrayLiteralType(std::vector<std::shared_ptr<Expr>> items) : Expr(NodeType::ArrayLiteral), items(items) {}

    std::vector<std::shared_ptr<Expr>> items;
};

struct ArrowFunctionType : public Expr {
    std::vector<std::shared_ptr<VarDeclarationType>> params;
    std::vector<std::shared_ptr<Stmt>> body;

    ArrowFunctionType(std::vector<std::shared_ptr<VarDeclarationType>> params, std::vector<std::shared_ptr<Stmt>> body) :
        Expr(NodeType::ArrowFunction), params(params), body(body) {}
};

struct NewExprType : public Expr {
    std::shared_ptr<Expr> constructor;
    std::vector<std::shared_ptr<Expr>> args;

    NewExprType(std::shared_ptr<Expr> constructor, std::vector<std::shared_ptr<Expr>> args) : Expr(NodeType::NewExpr), constructor(constructor), args(args) {}
};

struct CallExprType : public Expr {
    CallExprType(std::shared_ptr<Expr> calee = std::make_shared<Expr>(), std::vector<std::shared_ptr<Expr>> args = {}) : Expr(NodeType::CallExpr), args(args), calee(calee) {}
    std::vector<std::shared_ptr<Expr>> args;
    std::shared_ptr<Expr> calee;
};

struct MemberExprType : public Expr {
    MemberExprType(std::shared_ptr<Expr> object = std::make_shared<Expr>(), std::shared_ptr<Expr> property = std::make_shared<Expr>(), bool computed = false) : Expr(NodeType::MemberExpr), object(object), property(property), computed(computed) {}
    MemberExprType(std::shared_ptr<Expr> object, std::shared_ptr<Expr> property, bool computed, std::string lastProp) : Expr(NodeType::MemberExpr), object(object), property(property), computed(computed), lastProp(lastProp) {}
    std::shared_ptr<Expr> object;
    std::shared_ptr<Expr> property;
    std::string lastProp;
    bool computed;
};

struct MemberAssignmentType : public Expr {
    MemberAssignmentType(std::shared_ptr<Expr> obj, std::shared_ptr<Expr> property, std::shared_ptr<Expr> value, bool computed, std::string op) :
        Expr(NodeType::MemberAssignment), object(obj), newvalue(value),property(property), computed(computed), op(op) {}
    std::shared_ptr<Expr> object;
    std::shared_ptr<Expr> property;
    std::shared_ptr<Expr> newvalue;
    std::string op;
    bool computed;
};


struct BreakStmtType : public Stmt {
    BreakStmtType() : Stmt(NodeType::BreakStmt) {}
};

struct ContinueStmtType : public Stmt {
    ContinueStmtType() : Stmt(NodeType::ContinueStmt) {}
};

struct AwaitExprType : public Expr
{
    std::shared_ptr<Expr> caller;

    AwaitExprType(std::shared_ptr<Expr> caller)
        : Expr(NodeType::AwaitExpr), caller(caller) {}
};