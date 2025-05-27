#pragma once
#include <unordered_map>
#include <string>
#include <fstream>
#include <filesystem>
#include <unordered_set>
#include "ast.hpp"
#include "errors.hpp"
#include "context.hpp"
#include "parser.hpp"

namespace fs = std::filesystem;

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
};

struct Type;

struct TypeVal
{
    std::vector<VarDeclarationType*> params;
    std::unordered_map<std::string, Type> props;

    TypeVal(std::vector<VarDeclarationType*> params)
        : params(params) {}
    TypeVal(std::unordered_map<std::string, Type> props)
        : props(props) {}
    TypeVal() {}
};

struct Type
{
    TypeKind type;
    std::string name;
    TypeVal val;
    int typeID;
    std::string typeName;
    bool isInstance = false;

    Type(TypeKind type, std::string name, std::string typeName = "")
        : type(type), name(name), typeName(typeName) {}

    Type(TypeKind type, std::string name, bool isInstance)
        : type(type), name(name), isInstance(isInstance) {}

    Type(TypeKind type, std::string name, TypeVal value, std::string typeName = "")
        : type(type), name(name), val(value), typeName(typeName) {}
    
    Type()
        : type(TypeKind::Any), name("any") {}
};

inline std::unordered_map<std::string, Type> getTypedStdlib()
{
    return {
        {
            "http",
            Type(TypeKind::Module, "native module", TypeVal({
                {
                    "Server",
                    Type(TypeKind::Class, "native class", TypeVal({
                        {
                            "listen",
                            Type(TypeKind::Function, "native method", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "port", new IdentifierType("num")) })))
                        },
                        {
                            "get",
                            Type(TypeKind::Function, "native method", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "handler", new IdentifierType("function")) })))
                        },
                        {
                            "post",
                            Type(TypeKind::Function, "native method", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "handler", new IdentifierType("function")) })))
                        },
                        {
                            "put",
                            Type(TypeKind::Function, "native method", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "handler", new IdentifierType("function")) })))
                        },
                        {
                            "delete",
                            Type(TypeKind::Function, "native method", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "handler", new IdentifierType("function")) })))
                        },
                        {
                            "patch",
                            Type(TypeKind::Function, "native method", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "handler", new IdentifierType("function")) })))
                        },
                        {
                            "head",
                            Type(TypeKind::Function, "native method", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "handler", new IdentifierType("function")) })))
                        }
                    }), "httpServer")
                },
                {
                    "get",
                    Type(TypeKind::Function, "native function", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "url", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "req", new IdentifierType("map")) })))
                },
                {
                    "post",
                    Type(TypeKind::Function, "native function", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "url", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "req", new IdentifierType("map")) })))
                }
            }))
        },
        {
            "json",
            Type(TypeKind::Module, "native module", TypeVal({
                {
                    "parse",
                    Type(TypeKind::Function, "native function", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "raw", new IdentifierType("str")) })))
                },
                {
                    "stringify",
                    Type(TypeKind::Function, "native function", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "object") })))
                }
            }))
        },
        {
            "random",
            Type(TypeKind::Module, "native module", TypeVal({
                {
                    "randInt",
                    Type(TypeKind::Function, "native function", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "x"), new VarDeclarationType(new UndefinedLiteralType(), "y") })))
                },
                {
                    "rand",
                    Type(TypeKind::Function, "native function")
                }
            }))
        },
        {
            "fs",
            Type(TypeKind::Module, "native module", TypeVal({
                {
                    "readFile",
                    Type(TypeKind::Function, "native function", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")) })))
                },
                {
                    "writeFile",
                    Type(TypeKind::Function, "native function", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")), new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")) })))
                },
                {
                    "exists",
                    Type(TypeKind::Function, "native function", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")) })))
                },
                {
                    "isDirectory",
                    Type(TypeKind::Function, "native function", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")) })))
                },
                {
                    "listDir",
                    Type(TypeKind::Function, "native function", TypeVal(std::vector({ new VarDeclarationType(new UndefinedLiteralType(), "path", new IdentifierType("str")) })))
                }
            }))
        },
        {
            "date",
            Type(TypeKind::Module, "native module", TypeVal({
                {
                    "stamp",
                    Type(TypeKind::Function, "native function", TypeVal(std::vector({ new VarDeclarationType(new StringLiteralType("sec"), "unit", new IdentifierType("str")) })))
                }
            }))
        }
    };
}

class TypeEnv
{
public:
    TypeEnv(std::shared_ptr<TypeEnv> parent = nullptr);

    Type declareVar(std::string name, Type type);
    Type lookUp(std::string name);

    std::unordered_map<std::string, Type> getVars();
private:
    std::unordered_map<std::string, Type> m_variables;
    std::shared_ptr<TypeEnv> m_parent;
};

using TypeEnvPtr = std::shared_ptr<TypeEnv>;

inline std::unordered_set<std::string> boolOps = { "&&", "||", ">=", "<=", "<", ">", "!=", "==" };

class TC
{
public:
    void checkProgram(ProgramType* program, TypeEnvPtr env, Context* ctx = new Context());
private:
    int m_typeId = 0;

    Type check(Stmt* node, TypeEnvPtr env, Context* ctx = new Context());

    Type checkVarDecl(VarDeclarationType* decl, TypeEnvPtr env);
    Type checkAssign(AssignmentExprType* assign, TypeEnvPtr env);
    Type checkIdent(IdentifierType* ident, TypeEnvPtr env);
    Type checkProbe(ProbeDeclarationType* prb, TypeEnvPtr env);
    Type checkFunction(FunctionDeclarationType* fn, TypeEnvPtr env);
    Type checkCall(CallExprType* call, TypeEnvPtr env);
    Type checkObjectExpr(MapLiteralType* obj, TypeEnvPtr env);
    Type checkMemberExpr(MemberExprType* expr, TypeEnvPtr env);
    Type checkImportStmt(ImportStmtType* stmt, TypeEnvPtr env, Context* ctx);
    Type checkExportStmt(Stmt* stmt, TypeEnvPtr env, Context* ctx);
    Type checkBinExpr(BinaryExprType* expr, TypeEnvPtr env);
    Type checkClassDeclaration(ClassDefinitionType* cls, TypeEnvPtr env);
    Type checkNewExpr(NewExprType *expr, TypeEnvPtr env);

    std::unordered_map<std::string, Type> getExports(ProgramType* program, Context* ctx);

    Type getType(Expr* name, TypeEnvPtr env);
    bool compare(Type left, Type right, TypeEnvPtr env);
};