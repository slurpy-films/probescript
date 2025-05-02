#pragma once
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
    Error,
    NullLiteral,
    VarDeclaration,
    AssignmentExpr,
    PropertyLiteral,
    ObjectLiteral,
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
};

struct Stmt {
    NodeType kind;
    Stmt(NodeType kind) : kind(kind) {}
    virtual ~Stmt() = default;
    virtual std::string toString() const { return ""; }
    virtual std::string value() const { return "default"; }
};

struct ProgramType : public Stmt {
    ProgramType() : Stmt(NodeType::Program) {}
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

struct ImportStmtType : public Stmt {
    ImportStmtType(std::string module) : Stmt(NodeType::ImportStmt), module(module) {}
    std::string module;
};

struct ExportStmtType : public Stmt {
    ExportStmtType(Stmt* value) : Stmt(NodeType::ExportStmt), exporting(value) {}
    Stmt* exporting;
};

struct Expr : public Stmt { 
    Expr(NodeType kind = NodeType::NullLiteral) : Stmt(kind) {}

    std::string toString() const override {
        return value();
    }
};

struct WhileStmtType : public Stmt {
    WhileStmtType(Expr* condition, std::vector<Stmt*> body)
        : Stmt(NodeType::WhileStmt), condition(condition), body(body) {}
        Expr* condition;
        std::vector<Stmt*> body;
    };

struct ProbeDeclarationType : public Stmt {
    ProbeDeclarationType(std::string name, std::vector<Stmt*> body) : Stmt(NodeType::ProbeDeclaration), name(name), body(body) {}

    std::string name;
    std::vector<Stmt*> body;
};

struct VarDecalarationType : public Stmt {
    VarDecalarationType(Expr* value, std::string ident, bool constant = false) : Stmt(NodeType::VarDeclaration), value(value), identifier(ident), constant(constant) {}
    Expr* value;
    std::string identifier;
    bool constant;
};

struct IfStmtType : public Stmt {
    IfStmtType(Expr* condition, std::vector<Stmt*> body) : Stmt(NodeType::IfStmt), condition(condition), body(body) {}

    Expr* condition;
    std::vector<Stmt*> body;
    std::vector<Stmt*> elseStmt;
    bool hasElse = false;
};

struct AssignmentExprType : public Expr {
    AssignmentExprType(Expr* assigne, Expr* value, std::string op) : Expr(NodeType::AssignmentExpr), assigne(assigne), value(value), op(op) {}

    Expr* assigne;
    Expr* value;
    std::string op;
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

struct UndefinedLiteralType : public Expr {
    UndefinedLiteralType() : Expr(NodeType::UndefinedLiteral) {}
    std::string value() const override {
        return "undefined";
    };
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

struct ObjectLiteralType : public Expr {
    ObjectLiteralType(std::vector<PropertyLiteralType*> properties) : Expr(NodeType::ObjectLiteral), properties(properties) {}
    std::string value() const override {
        return "null";
    };

    std::vector<PropertyLiteralType*> properties;
};

struct ArrayLiteralType : public Expr {
    ArrayLiteralType(std::vector<Expr*> items) : Expr(NodeType::ArrayLiteral), items(items) {}

    std::vector<Expr*> items;
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
    Expr* object;
    Expr* property;
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

struct FunctionDeclarationType : public Stmt {
    FunctionDeclarationType(std::vector<std::string> params, std::string name, std::vector<Stmt*> body) : Stmt(NodeType::FunctionDeclaration), parameters(params), name(name), body(body) {}

    std::vector<std::string> parameters;
    std::string name;
    std::vector<Stmt*> body;
};