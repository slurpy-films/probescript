#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "frontend/ast.hpp"
#include "frontend/lexer.hpp"

namespace Probescript::Typechecker
{

class TypeEnv;
using TypeEnvPtr = std::shared_ptr<TypeEnv>;

struct Type;
using TypePtr = std::shared_ptr<Type>;

} // namespace Probescript::Typechecker

#include "context.hpp"

namespace Probescript::Typechecker
{
enum class TypeKind
{
    Number,
    String,
    Any,
    Function,
    Custom,
    Object,
    Undefined,
    Bool,
    Class,
    Probe,
    Module, // A module is handled like an object, but if the user tries to access a member it doesn't have, it throws the process
    Array,
    Future,
};

struct Parameter;

struct TypeVal
{
    std::vector<std::shared_ptr<Parameter>> params = {};
    std::vector<std::shared_ptr<AST::VarDeclarationType>> templateparams = {};
    std::unordered_map<std::string, TypePtr> props = {};
    TypePtr returntype;

    bool isAsync = false;

    // Used for re-checking the source of a template function
    std::shared_ptr<AST::Stmt> sourcenode;
    TypeEnvPtr declenv;

    TypePtr futureVal; // What this future eventually will resolve to

    TypeVal(bool _, TypePtr futureVal) // bool _ so there is a defference between this and the returntype constructor
        : futureVal(futureVal) {}

    TypeVal(std::vector<std::shared_ptr<Parameter>> params)
        : params(params) {}
    TypeVal(TypePtr returntype)
        : returntype(returntype) {}
    TypeVal(std::vector<std::shared_ptr<Parameter>> params, TypePtr returntype)
        : params(params), returntype(returntype) {}
    TypeVal(std::vector<std::shared_ptr<Parameter>> params, TypePtr returntype, std::vector<std::shared_ptr<AST::VarDeclarationType>> templateparams)
        : params(params), returntype(returntype), templateparams(templateparams) {}
    TypeVal(std::unordered_map<std::string, TypePtr> props)
        : props(props) {}
    TypeVal() {}
};

using TypeValPtr = std::shared_ptr<TypeVal>;

struct Type
{
    TypeKind type;
    std::string name;
    TypeValPtr val = std::make_shared<TypeVal>();
    std::string typeName;
    bool isInstance = false;
    bool templateSub = false;
    TypePtr parent;

    Type(TypeKind type, std::string name, std::string typeName = "")
        : type(type), name(name), typeName(typeName) {}

    Type(TypeKind type, std::string name, bool isInstance)
        : type(type), name(name), isInstance(isInstance) {}

    Type(TypeKind type, std::string name, TypeValPtr value, std::string typeName = "")
        : type(type), name(name), val(value), typeName(typeName) {}
    
    Type(const TypePtr& other)
        : type(other->type),
        name(other->name),
        val(other->val),
        typeName(other->typeName),
        isInstance(other->isInstance),
        templateSub(other->templateSub),
        parent(other->parent) {}

    Type()
        : type(TypeKind::Any), name("any") {}
};

// Global types
extern TypePtr g_anyty;
extern TypePtr g_numty;
extern TypePtr g_strty;
extern TypePtr g_boolty;
extern TypePtr g_mapty;
extern TypePtr g_arrayty;

struct Parameter
{
    std::string ident;
    TypePtr type = g_anyty;
    Lexer::Token token;

    bool hasDefault;

    Parameter(std::string ident, TypePtr type, Lexer::Token token = Lexer::Token(), bool hasDefault = false)
        : ident(ident), type(type), token(token), hasDefault(hasDefault) {}

    Parameter(std::string ident, TypePtr type, bool hasDefault)
        : ident(ident), type(type), hasDefault(hasDefault) {}

    Parameter(std::string ident)
        : ident(ident), type(g_anyty) {}
};

class TypeEnv
{
public:
    TypeEnv(std::shared_ptr<TypeEnv> parent = nullptr);

    TypePtr declareVar(std::string name, TypePtr type, Lexer::Token tk);
    TypePtr lookUp(std::string name, Lexer::Token tk);

    void massDeclare(std::unordered_map<std::string, TypePtr> vars);

    std::unordered_map<std::string, TypePtr> getVars();
    std::shared_ptr<Context> m_ctx;
private:
    bool m_ready = false;

    std::unordered_map<std::string, TypePtr> m_variables;
    std::shared_ptr<TypeEnv> m_parent;
};

} // namespace Probescript::Typechecker