#pragma once
#include <vector>
#include <string>
#include <iostream>
#include <memory>

using namespace std;

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
};

struct Stmt {
    NodeType kind;
    Stmt(NodeType kind) : kind(kind) {}
    virtual ~Stmt() = default;
    virtual string toString() const { return ""; }
    virtual string value() const { return "default"; }
};

struct ProgramType : public Stmt {
    ProgramType() : Stmt(NodeType::Program) {}
    vector<Stmt*> body;
    
    ~ProgramType() {
        for (auto stmt : body) {
            delete stmt;
        }
    }
};

struct ImportStmtType : public Stmt {
    ImportStmtType(string module) : Stmt(NodeType::ImportStmt), module(module) {}
    string module;
};

struct ExportStmtType : public Stmt {
    ExportStmtType(Stmt* value) : Stmt(NodeType::ExportStmt), exporting(value) {}
    Stmt* exporting;
};

struct Expr : public Stmt { 
    Expr(NodeType kind = NodeType::NullLiteral) : Stmt(kind) {}

    string toString() const override {
        return value();
    }
};

struct ProbeDeclarationType : public Stmt {
    ProbeDeclarationType(string name, vector<Stmt*> body) : Stmt(NodeType::ProbeDeclaration), name(name), body(body) {}

    string name;
    vector<Stmt*> body;
};

struct VarDecalarationType : public Stmt {
    VarDecalarationType(Expr* value, string ident, bool constant = false) : Stmt(NodeType::VarDeclaration), value(value), identifier(ident), constant(constant) {}
    Expr* value;
    string identifier;
    bool constant;
};

struct IfStmtType : public Stmt {
    IfStmtType(Expr* condition, vector<Stmt*> body) : Stmt(NodeType::IfStmt), condition(condition), body(body) {}

    Expr* condition;
    vector<Stmt*> body;
};

struct AssignmentExprType : public Expr {
    AssignmentExprType(Expr* assigne = new Expr(), Expr* value = new Expr()) : Expr(NodeType::AssignmentExpr), assigne(assigne), value(value) {}

    Expr* assigne;
    Expr* value;
};

struct BinaryExprType : public Expr {
    BinaryExprType(Expr* left, Expr* right, const string& op) 
        : Expr(NodeType::BinaryExpr), left(left), right(right), op(op) {}
    
    ~BinaryExprType() {
        delete left;
        delete right;
    }

    Expr* left;
    Expr* right;
    string op;
    
    string value() const override {
        return left->toString() + " " + op + " " + right->toString();
    }
};

struct IdentifierType : public Expr {
    IdentifierType(const string& symbol) 
        : Expr(NodeType::Identifier), symbol(symbol) {}

    string symbol;
    
    string value() const override {
        return symbol;
    }
};

struct NumericLiteralType : public Expr {
    NumericLiteralType(double val) 
        : Expr(NodeType::NumericLiteral), numValue(val) {}
    
    double numValue;
    
    string value() const override {
        return to_string(numValue);
    }
};

struct StringLiteralType : public Expr {
    string strValue;
    StringLiteralType(string val) : Expr(NodeType::StringLiteral), strValue(val) {}

    string value() const override {
        return strValue;
    }
};

struct NullLiteralType : public Expr {
    NullLiteralType() : Expr(NodeType::NullLiteral) {}
    string value() const override {
        return "null";
    };
};

struct UndefinedLiteralType : public Expr {
    UndefinedLiteralType() : Expr(NodeType::UndefinedLiteral) {}
    string value() const override {
        return "undefined";
    };
};

struct PropertyLiteralType : public Expr {
    PropertyLiteralType(string key, Expr* val) : Expr(NodeType::PropertyLiteral), key(key), val(val) {}
    string value() const override {
        return "null";
    };

    string key;
    Expr* val;

};

struct ObjectLiteralType : public Expr {
    ObjectLiteralType(vector<PropertyLiteralType*> properties) : Expr(NodeType::ObjectLiteral), properties(properties) {}
    string value() const override {
        return "null";
    };

    vector<PropertyLiteralType*> properties;
};

struct ArrayLiteralType : public Expr {
    ArrayLiteralType(vector<Expr*> items) : Expr(NodeType::ArrayLiteral), items(items) {}

    vector<Expr*> items;
};

struct CallExprType : public Expr {
    CallExprType(Expr* calee = new Expr(), vector<Expr*> args = {}) : Expr(NodeType::CallExpr), args(args), calee(calee) {}
    vector<Expr*> args;
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
    FunctionDeclarationType(vector<string> params, string name, vector<Stmt*> body) : Stmt(NodeType::FunctionDeclaration), parameters(params), name(name), body(body) {}

    vector<string> parameters;
    string name;
    vector<Stmt*> body;
};