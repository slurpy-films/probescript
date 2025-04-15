#pragma once
#include "values.hpp"
#include "../ast.hpp"
#include <cmath>
#include <string>
#include "env.cpp"

RuntimeVal* eval(Stmt* astNode, Env* env);

RuntimeVal* evalProgram(ProgramType* program, Env* env) {
    RuntimeVal* lastEval = new NullVal();

    for (Stmt* stmt : program->body) {
        lastEval = eval(stmt, env);
    }

    return lastEval;
}

RuntimeVal* evalAssignment(AssignmentExprType* assignment, Env* env) {
    if (assignment->assigne->kind != NodeType::Identifier) {
        cout << "Expected Identifier" << endl;
        exit(1);
    }

    string varName = static_cast<IdentifierType*>(assignment->assigne)->symbol;

    return env->assignVar(varName, eval(assignment->value, env));
}

RuntimeVal* evalFunctionDeclaration(FunctionDeclarationType* declaration, Env* env) {
    FunctionValue* fn = new FunctionValue(declaration->name, declaration->parameters, env, declaration->body);

    return env->declareVar(declaration->name, fn, true);
}

NumberVal* evalNumericBinExpr(NumberVal* lhs, NumberVal* rhs, string op) {
    double result = 0;

    double left = std::stod(lhs->value);
    double right = std::stod(rhs->value);
    
    if (op == "+") {
        result = left + right;
    } else if (op == "-") {
        result = left - right;
    } else if (op == "*") {
        result = left * right;
    } else if (op == "/") {
        result = left / right;
    } else if (op == "%") {
        result = fmod(left, right);
    }

    return new NumberVal(std::to_string(result));
}

RuntimeVal* evalMemberExpr(MemberExprType* expr, Env* env) {
    RuntimeVal* obj = eval(expr->object, env);

    if (obj->type != ValueType::Object) {
        cerr << "Cannot find member of non object";
        exit(1);
    }

    string key;

    if (expr->computed) {
        RuntimeVal* propValue = eval(expr->property, env);

        if (propValue->type != ValueType::String) {
            cerr << "Computed property must evaluate to a string";
            exit(1);
        }

        key = static_cast<StringVal*>(propValue)->string;
    } else {
        IdentifierType* ident = static_cast<IdentifierType*>(expr->property);
        key = ident->symbol;
    }

    ObjectVal* objectVal = static_cast<ObjectVal*>(obj);

    string objName = static_cast<IdentifierType*>(expr->object)->symbol;

    ObjectVal* object = static_cast<ObjectVal*>(env->lookupVar(objName));

    

    if (object->properties.count(key) == 0) {
        cerr << "Object has no property " << key;
        exit(1);
    }

    return object->properties[key];
}

RuntimeVal* evalBinExpr(BinaryExprType* binop, Env* env) {
    RuntimeVal* left = eval(binop->left, env);
    RuntimeVal* right = eval(binop->right, env);

    if (left->type == ValueType::Number && right->type == ValueType::Number) {
        return evalNumericBinExpr(static_cast<NumberVal*>(left), static_cast<NumberVal*>(right), binop->op);
    }

    return new NullVal();
}

RuntimeVal* evalIdent(IdentifierType* ident, Env* env) {
    RuntimeVal* value = env->lookupVar(ident->symbol);
    return value;
}

RuntimeVal* evalObject(ObjectLiteralType* obj, Env* env) {
    ObjectVal* object = new ObjectVal();
    for (PropertyLiteralType* property : obj->properties) {
        RuntimeVal* runtimeval = (property->val == nullptr) ? env->lookupVar(property->key) : eval(property->val, env);
        object->properties[property->key] = runtimeval;
    }

    return object;
}

RuntimeVal* evalCall(CallExprType* call, Env* env) {
    vector<RuntimeVal*> args;

    for (auto arg : call->args) {
        args.push_back(eval(arg, env));
    };

    RuntimeVal* fn = eval(call->calee, env);

    if (fn == nullptr) {
        cerr << "Error: function call target is null";
        exit(1);
    }

    if (fn->type == ValueType::NativeFn) {

        RuntimeVal* result = static_cast<NativeFnValue*>(fn)->call(args, env);

        return result;
    }

    if (fn->type == ValueType::Function) {
        FunctionValue* func = static_cast<FunctionValue*>(fn);
        Env* scope = new Env(func->declarationEnv);

        for (int i = 0; i < func->params.size(); i++) {
            string varname = func->params[i];
            scope->declareVar(varname, args[i], false);
        }

        RuntimeVal* result = new NullVal();

        for (Stmt* stmt : func->body) {
            result = eval(stmt, scope);
        }

        return result;
    }

    cerr << "Cannot call value that is not a function, " << fn->type;
    exit(1);
}


RuntimeVal* evalVarDeclaration(VarDecalarationType* var, Env* env, bool constant = false) {
    RuntimeVal* value = var->value != nullptr ? eval(var->value, env) : new NullVal();
    return env->declareVar(var->identifier, value, constant);
}

RuntimeVal* eval(Stmt* astNode, Env* env) {
    switch (astNode->kind) {
        case NodeType::NumericLiteral: {
            auto num = static_cast<NumericLiteralType*>(astNode);
            return new NumberVal(num->value());
        }

        case NodeType::BinaryExpr:
            return evalBinExpr(static_cast<BinaryExprType*>(astNode), env);

        case NodeType::Program:
            return evalProgram(static_cast<ProgramType*>(astNode), env);

        case NodeType::NullLiteral:
            return new NullVal();

        case NodeType::Identifier:
            return evalIdent(static_cast<IdentifierType*>(astNode), env);

        case NodeType::ObjectLiteral:
            return evalObject(static_cast<ObjectLiteralType*>(astNode), env);

        case NodeType::CallExpr:
            return evalCall(static_cast<CallExprType*>(astNode), env);

        case NodeType::VarDeclaration:
            return evalVarDeclaration(static_cast<VarDecalarationType*>(astNode), env);

        case NodeType::FunctionDeclaration:
            return evalFunctionDeclaration(static_cast<FunctionDeclarationType*>(astNode), env);

        case NodeType::AssignmentExpr:
            return evalAssignment(static_cast<AssignmentExprType*>(astNode), env);

        case NodeType::MemberExpr:
            return evalMemberExpr(static_cast<MemberExprType*>(astNode), env);

        default:
            cout << "Unexpected AST-node kind found: ";
            cout << astNode->kind << endl;
            return new NullVal();
    }
}