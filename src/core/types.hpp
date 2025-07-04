#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>
#include "frontend/ast.hpp"

class TypeEnv;
using TypeEnvPtr = std::shared_ptr<TypeEnv>;

struct Type;
using TypePtr = std::shared_ptr<Type>;

#include "context.hpp"

struct Context;

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
};

struct TypeVal
{
    std::vector<std::shared_ptr<VarDeclarationType>> params = {};
    std::vector<std::shared_ptr<VarDeclarationType>> templateparams = {};
    std::unordered_map<std::string, TypePtr> props = {};
    TypePtr returntype;

    // Used for re-checking the source of a template function
    std::shared_ptr<Stmt> sourcenode;
    TypeEnvPtr declenv;

    TypeVal(std::vector<std::shared_ptr<VarDeclarationType>> params)
        : params(params) {}
    TypeVal(TypePtr returntype)
        : returntype(returntype) {}
    TypeVal(std::vector<std::shared_ptr<VarDeclarationType>> params, TypePtr returntype)
        : params(params), returntype(returntype) {}
    TypeVal(std::vector<std::shared_ptr<VarDeclarationType>> params, TypePtr returntype, std::vector<std::shared_ptr<VarDeclarationType>> templateparams)
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