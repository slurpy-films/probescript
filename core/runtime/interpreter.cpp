#include "runtime/interpreter.hpp"

using namespace Probescript;
using namespace Probescript::Interpreter;

Values::Val Interpreter::evalArray(std::shared_ptr<AST::ArrayLiteralType> expr, EnvPtr env) {
    std::vector<Values::Val> items;

    for (std::shared_ptr<AST::Expr> item : expr->items)
    {
        items.push_back(eval(item, env));
    }

    return Values::makeVal<Values::ArrayVal>(expr->token, items);
}

Values::Val Interpreter::evalArrowFunction(std::shared_ptr<AST::ArrowFunctionType> fn, EnvPtr env) {
    return Values::makeVal<Values::FunctionValue>(fn->token, "arrow", fn->params, env, fn->body);
}

Values::Val Interpreter::evalTemplateCall(std::shared_ptr<AST::TemplateCallType> call, EnvPtr env)
{
    Values::Val caller = eval(call->caller, env);
    EnvPtr scope = std::make_shared<Env>(env);
    std::string name = "template";
    std::vector<std::shared_ptr<AST::VarDeclarationType>> params;
    std::vector<std::shared_ptr<AST::Stmt>> body;

    if (caller->type == Values::ValueType::Function)
    {
        std::shared_ptr<Values::FunctionValue> fn = std::static_pointer_cast<Values::FunctionValue>(caller);
        scope = fn->declarationEnv;
        name = fn->name;
        params = fn->params;
        body = fn->body;

        for (size_t i = 0; i < fn->templateparams.size(); i++)
        {
            scope->declareVar(
                fn->templateparams[i]->identifier, 
                (call->templateArgs.size() > i 
                    ? eval(call->templateArgs[i], scope) 
                    : std::make_shared<Values::UndefinedVal>()),
                fn->templateparams[i]->token
            );
        }
    }

    return Values::makeVal<Values::FunctionValue>(call->token, name, params, scope, body);
}

Values::Val Interpreter::evalAssignment(std::shared_ptr<AST::AssignmentExprType> assignment, EnvPtr env)
{
    if (assignment->assigne->kind != AST::NodeType::Identifier)
    {
        throw ThrowException(CustomError("Expected Identifier in assignment", "AssignmentError", assignment->token));
    }

    std::string varName = std::static_pointer_cast<AST::IdentifierType>(assignment->assigne)->symbol;

    Values::Val leftVal = eval(assignment->assigne, env);
    Values::Val rightVal = eval(assignment->value, env);

    if (assignment->op == "=")
    {
        return env->assignVar(varName, rightVal, assignment->token);
    }

    Values::Val result;

    if (assignment->op == "-=") result = leftVal->sub(rightVal);
    else if (assignment->op == "*=") result = leftVal->mul(rightVal);
    else if (assignment->op == "/=") result = leftVal->div(rightVal);
    else if (assignment->op == "+=") result = leftVal->add(rightVal);
    else {
        throw ThrowException(CustomError("Unsupported assignment operator: " + assignment->op, "AssignmentError", assignment->token));
    }

    return env->assignVar(varName, result, assignment->token);
}

Values::Val Interpreter::evalUnaryPostfix(std::shared_ptr<AST::UnaryPostFixType> expr, EnvPtr env)
{
    if (expr->assigne->kind == AST::NodeType::Identifier)
    {
        std::string varName = std::static_pointer_cast<AST::IdentifierType>(expr->assigne)->symbol;
        Values::Val current = env->lookupVar(varName, expr->assigne->token);

        if (current->type != Values::ValueType::Number)
        {
            throw ThrowException(CustomError("Postfix operators only supported on numbers", "OperatorError", current->token));
        }

        double value = std::static_pointer_cast<Values::NumberVal>(current)->toNum();
        double newValue = value;

        if (expr->op == "++") newValue = value + 1;
        else if (expr->op == "--") newValue = value - 1;
        else
        {
            throw ThrowException(CustomError("Unknown postfix operator: " + expr->op, "OperatorError", expr->token));
        }

        env->assignVar(varName, Values::makeVal<Values::NumberVal>(expr->token, newValue), expr->token);

        return Values::makeVal<Values::NumberVal>(expr->token, value);
    } else if (expr->assigne->kind == AST::NodeType::MemberExpr) {
        auto member = std::make_shared<AST::MemberAssignmentType>(
            std::static_pointer_cast<AST::MemberExprType>(expr->assigne)->object,
            std::static_pointer_cast<AST::MemberExprType>(expr->assigne)->property,
            std::make_shared<AST::NumericLiteralType>(1),
            std::static_pointer_cast<AST::MemberExprType>(expr->assigne)->computed,
            expr->op
        );
        member->token = expr->token;

        return evalMemberAssignment(member, env);
    }

    return std::make_shared<Values::UndefinedVal>();
}


Values::Val Interpreter::evalUnaryPrefix(std::shared_ptr<AST::UnaryPrefixType> expr, EnvPtr env) {
    Values::Val val = eval(expr->assigne, env);

    if (expr->op == "!")
    {
        return Values::makeVal<Values::BooleanVal>(expr->token, !val->toBool());
    }

    return std::make_shared<Values::UndefinedVal>();
}


std::unordered_set<std::string> booleanOperators = { "&&", "||", ">=", "<=", "<", ">", "!=", "==" };

Values::Val Interpreter::evalBinExpr(std::shared_ptr<AST::BinaryExprType> binop, EnvPtr env) {
    if (booleanOperators.count(binop->op))
    {
        return evalBooleanBinExpr(binop, env);
    }

    Values::Val left = eval(binop->left, env);
    Values::Val right = eval(binop->right, env);

    std::string op = binop->op;
    if (op == "+") {
        return left->add(right);
    } else if (op == "-") {
        return left->sub(right);
    } else if (op == "*") {
        return left->mul(right);
    } else if (op == "/") {
        return left->div(right);
    } else if (op == "%") {
        return left->mod(right);
    }

    throw ThrowException(CustomError("Invalid operants: " + left->toString() + " and " + right->toString(), "OperatorError", binop->token));
}

Values::Val Interpreter::evalBody(std::vector<std::shared_ptr<AST::Stmt>> body, EnvPtr env, bool isLoop)
{
    for (std::shared_ptr<AST::Stmt> stmt : body)
    {
        eval(stmt, env);
    }

    return std::make_shared<Values::UndefinedVal>();
}

Values::Val Interpreter::evalTernaryExpr(std::shared_ptr<AST::TernaryExprType> expr, EnvPtr env)
{
    Values::Val cond = eval(expr->cond, env);

    if (cond->toBool())
        return eval(expr->cons, env);
    else
        return eval(expr->alt, env);
}

Values::Val Interpreter::evalBooleanBinExpr(std::shared_ptr<AST::BinaryExprType> binop, EnvPtr env)
{
    Values::Val left = eval(binop->left, env);
    Values::Val right = eval(binop->right, env);

    const std::string& op = binop->op;

    if (op == "&&" || op == "||")
    {
        bool l = left->toBool();
        bool r = right->toBool();
        return Values::makeVal<Values::BooleanVal>(binop->token, ((op == "&&") ? (l && r) : (l || r)));
    }

    if (op == "==" || op == "!=")
    {
        bool result = *left == *right;
        return Values::makeVal<Values::BooleanVal>(binop->token, op == "==" ? result : !result);
    }

    if (op == "<" || op == ">" || op == "<=" || op == ">=")
    {
        double l = left->toNum();
        double r = right->toNum();
        bool result = false;

        if (op == "<") result = l < r;
        else if (op == ">") result = l > r;
        else if (op == "<=") result = l <= r;
        else if (op == ">=") result = l >= r;

        return Values::makeVal<Values::BooleanVal>(binop->token, result);
    }

    throw ThrowException(CustomError("Invalid binary boolean operator: " + op, "OperatorError", binop->token));
}

Values::Val Interpreter::evalFunctionDeclaration(std::shared_ptr<AST::FunctionDeclarationType> declaration, EnvPtr env, bool onlyValue)
{
    std::shared_ptr<Values::FunctionValue> fn = Values::makeVal<Values::FunctionValue>(declaration->token, declaration->name, declaration->parameters, env, declaration->body, declaration->isAsync);
    fn->templateparams = declaration->templateparams;

    return onlyValue ? fn : env->declareVar(declaration->name, fn, declaration->token);
}

Values::Val Interpreter::evalIdent(std::shared_ptr<AST::IdentifierType> ident, EnvPtr env)
{
    Values::Val value = env->lookupVar(ident->symbol, ident->token);
    return value;
}

Values::Val Interpreter::evalIfStmt(std::shared_ptr<AST::IfStmtType> stmt, EnvPtr baseEnv)
{
    Values::Val condition = eval(stmt->condition, baseEnv);

    bool cond = condition->toBool();

    if (cond)
    {
        EnvPtr env = std::make_shared<Env>(baseEnv);
        return evalBody(stmt->body, env);
    }
    else if (stmt->hasElse)
    {
        EnvPtr env = std::make_shared<Env>(baseEnv);
        return evalBody(stmt->elseStmt, env);
    }

    return std::make_shared<Values::UndefinedVal>();
}

Values::Val Interpreter::evalImportStmt(std::shared_ptr<AST::ImportStmtType> importstmt, EnvPtr envptr, std::shared_ptr<Context> context)
{
    std::string modulename = importstmt->name;

    std::unordered_map<std::string, Values::Val> stdlib;

    for (const auto& [key, pair] : g_stdlib)
        stdlib[key] = pair.first;
        
    if (stdlib.find(modulename) != stdlib.end())
    {
        if (importstmt->hasMember)
        {
            std::shared_ptr<AST::Expr> member = importstmt->module;
            EnvPtr modEnv = std::make_shared<Env>();
            modEnv->declareVar(modulename, stdlib[modulename], member->token);
            envptr->declareVar(importstmt->customIdent ? importstmt->ident : std::static_pointer_cast<AST::MemberExprType>(importstmt->module)->lastProp, eval(member, modEnv), member->token);
        }
        else envptr->declareVar(importstmt->customIdent ? importstmt->ident : modulename, stdlib[modulename], importstmt->token);
        return std::make_shared<Values::UndefinedVal>();
    }

    if (context->modules.find(modulename) == context->modules.end())
    {
        throw ThrowException(CustomError("Cannot find module " + modulename, "ImportError", importstmt->token));
    }

    fs::path filepath = context->modules[modulename];

    std::ifstream stream(filepath);

    std::string file((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

    Parser parser;

    std::shared_ptr<AST::ProgramType> program = parser.parse(file);

    std::shared_ptr<Context> conf = std::make_shared<Context>(RuntimeType::Exports);

    conf->modules = context->modules;

    Values::Val evaluated = eval(program, std::make_shared<Env>(), conf);

    std::shared_ptr<Values::ObjectVal> moduleObj = Values::makeVal<Values::ObjectVal>(importstmt->token, evaluated->exports);

    if (importstmt->hasMember)
    {
        std::shared_ptr<AST::Expr> member = importstmt->module;
        EnvPtr modEnv = std::make_shared<Env>();
        modEnv->declareVar(modulename, moduleObj, member->token);
        envptr->declareVar(importstmt->customIdent ? importstmt->ident : std::static_pointer_cast<AST::MemberExprType>(importstmt->module)->lastProp, eval(member, modEnv), member->token);
    }
    else envptr->declareVar(importstmt->customIdent ? importstmt->ident : modulename, moduleObj, importstmt->token);

    return std::make_shared<Values::UndefinedVal>();
}

Values::Val Interpreter::evalMemberAssignment(std::shared_ptr<AST::MemberAssignmentType> expr, EnvPtr env)
{
    Values::Val obj = eval(expr->object, env);
    Values::Val value = eval(expr->newvalue, env);

    std::string key;

    if (expr->computed)
    {
        Values::Val propValue = eval(expr->property, env);

        if (propValue->type == Values::ValueType::Number)
        {
            int index = propValue->toNum();

            if (obj->type == Values::ValueType::Array)
            {
                std::shared_ptr<Values::ArrayVal> array = std::static_pointer_cast<Values::ArrayVal>(obj);

                if (index >= array->items.size())
                {
                    array->items.resize(index + 1, std::make_shared<Values::UndefinedVal>());
                }

                if (expr->op == "=")
                {
                    array->items[index] = value;
                }
                else if (expr->op == "+=")
                {
                    array->items[index] = array->items[index]->add(value);
                }
                else if (expr->op == "-=")
                {
                    array->items[index] = array->items[index]->sub(value);
                }
                else if (expr->op == "*=")
                {
                    array->items[index] = array->items[index]->mul(value);
                }
                else if (expr->op == "/=")
                {
                    array->items[index] = array->items[index]->div(value);
                }
                else if (expr->op == "++")
                {
                    array->items[index] = array->items[index]->add(value);
                }
                else if (expr->op == "--")
                {
                    array->items[index] = array->items[index]->sub(value);
                }
                return array;
            }
            else
            {
                throw ThrowException(CustomError("Cannot use numeric index on non-array object", "MemberError", propValue->token));
            }
        }

        if (propValue->type != Values::ValueType::String)
        {
            throw ThrowException(CustomError("Computed property must evaluate to a string or number", "MemberError", expr->token));
        }

        key = std::static_pointer_cast<Values::StringVal>(propValue)->string;
    }
    else
    {
        std::shared_ptr<AST::IdentifierType> ident = std::static_pointer_cast<AST::IdentifierType>(expr->property);
        key = ident->symbol;
    }

    if (obj->type == Values::ValueType::Object)
    {
        std::shared_ptr<Values::ObjectVal> objectVal = std::static_pointer_cast<Values::ObjectVal>(obj);
        if (expr->op == "=")
        {
            objectVal->properties[key] = value;
        }
        else if (expr->op == "+=")
        {
            objectVal->properties[key] = objectVal->properties[key]->add(value);
        }
        else if (expr->op == "-=")
        {
            objectVal->properties[key] = objectVal->properties[key]->sub(value);
        }
        else if (expr->op == "*=")
        {
            objectVal->properties[key] = objectVal->properties[key]->mul(value);
        }
        else if (expr->op == "/=")
        {
            objectVal->properties[key] = objectVal->properties[key]->div(value);
        }
        else if (expr->op == "++")
        {
            objectVal->properties[key] = objectVal->properties[key]->add(value);
        }
        else if (expr->op == "--")
        {
            objectVal->properties[key] = objectVal->properties[key]->sub(value);
        }
        return objectVal;
    }

    throw ThrowException(CustomError("Cannot assign member to non-object/non-array value", "TypeError", expr->token));
}

Values::Val Interpreter::evalMemberExpr(std::shared_ptr<AST::MemberExprType> expr, EnvPtr env)
{
    Values::Val obj = eval(expr->object, env);
    
    if (obj->type != Values::ValueType::Array || !expr->computed)
    {
        std::string key;

        if (expr->computed)
        {
            Values::Val propValue = eval(expr->property, env);
    
            if (propValue->type != Values::ValueType::String)
            {
                throw ThrowException(CustomError("Computed property must evaluate to a string", "TypeError", expr->token));
            }
    
            key = std::static_pointer_cast<Values::StringVal>(propValue)->string;
        }
        else
        {
            std::shared_ptr<AST::IdentifierType> ident = std::static_pointer_cast<AST::IdentifierType>(expr->property);
            key = ident->symbol;
        }

        Values::Val object = obj;

        if (object->properties.count(key) == 0)
        {
            return std::make_shared<Values::UndefinedVal>();
        }

        return object->properties[key];
    }
    else
    {
        Values::Val indexval = eval(expr->property, env);

        if (indexval->type != Values::ValueType::Number)
        {
            throw ThrowException(CustomError("Array index must evaluate to a number", "TypeError", expr->token));
        }

        std::shared_ptr<Values::NumberVal> index = std::static_pointer_cast<Values::NumberVal>(indexval);

        std::shared_ptr<Values::ArrayVal> array = std::static_pointer_cast<Values::ArrayVal>(obj);

        int idx = index->number;

        if (idx < 0 || idx >= static_cast<int>(array->items.size()))
        {
            return std::make_shared<Values::UndefinedVal>();
        }

        return array->items[idx];
    }
}

Values::Val Interpreter::evalObject(std::shared_ptr<AST::MapLiteralType> obj, EnvPtr env)
{
    std::shared_ptr<Values::ObjectVal> object = std::make_shared<Values::ObjectVal>();
    for (std::shared_ptr<AST::PropertyLiteralType> property : obj->properties)
    {
        Values::Val runtimeval = (property->val == nullptr) ? env->lookupVar(property->key, property->token) : eval(property->val, env);
        object->properties[property->key] = runtimeval;
    }

    return object;
}

Values::Val Interpreter::evalVarDeclaration(std::shared_ptr<AST::VarDeclarationType> decl, EnvPtr env, bool constant)
{
    Values::Val value = decl->value != nullptr ? eval(decl->value, env) : std::make_shared<Values::UndefinedVal>();
    return env->declareVar(decl->identifier, value, decl->token);
}

Values::Val Interpreter::evalThrowStmt(std::shared_ptr<AST::ThrowStmtType> stmt, EnvPtr env)
{
    throw ThrowException(eval(stmt->err, env)->toString());
}

Values::Val Interpreter::evalTryStmt(std::shared_ptr<AST::TryStmtType> stmt, EnvPtr env)
{
    std::shared_ptr<Values::FunctionValue> fn = Values::makeVal<Values::FunctionValue>(stmt->catchHandler->token, "catch", stmt->catchHandler->parameters, env, stmt->catchHandler->body);
    EnvPtr scope = std::make_shared<Env>(env);

    try
    {
        evalBody(stmt->body, scope);
    }
    catch (const ThrowException& e)
    {
        evalCallWithFnVal(fn, { std::make_shared<Values::StringVal>(e.what()) }, scope);
    }

    return std::make_shared<Values::UndefinedVal>();
}

Values::Val Interpreter::eval(std::shared_ptr<AST::Stmt> astNode, EnvPtr env, std::shared_ptr<Context> context)
{
    switch (astNode->kind)
    {
        case AST::NodeType::NumericLiteral:
        {
            std::shared_ptr<AST::NumericLiteralType> num = std::static_pointer_cast<AST::NumericLiteralType>(astNode);
            return Values::makeVal<Values::NumberVal>(astNode->token, num->value());
        }

        case AST::NodeType::StringLiteral:
        {
            std::shared_ptr<AST::StringLiteralType> str = std::static_pointer_cast<AST::StringLiteralType>(astNode);
            return Values::makeVal<Values::StringVal>(astNode->token, str->strValue);
        }

        case AST::NodeType::UndefinedLiteral:
        {
            return std::make_shared<Values::UndefinedVal>();
        }

        case AST::NodeType::BoolLiteral:
            return Values::makeVal<Values::BooleanVal>(astNode->token, std::static_pointer_cast<AST::BoolLiteralType>(astNode)->value);

        case AST::NodeType::TryStmt:
            return evalTryStmt(std::static_pointer_cast<AST::TryStmtType>(astNode), env);

        case AST::NodeType::ThrowStmt:
            return evalThrowStmt(std::static_pointer_cast<AST::ThrowStmtType>(astNode), env);

        case AST::NodeType::ReturnStmt:
        {
            Values::Val value = eval(std::static_pointer_cast<AST::ReturnStmtType>(astNode)->val, env);
            throw Values::ReturnSignal(value, CustomError("Did not expect return statement", "ReturnError", astNode->token));
        }
        
        case AST::NodeType::ClassDefinition:
            return evalClassDefinition(std::static_pointer_cast<AST::ClassDefinitionType>(astNode), env);

        case AST::NodeType::ProbeDeclaration:
            return evalProbeDeclaration(std::static_pointer_cast<AST::ProbeDeclarationType>(astNode), env);
            
        case AST::NodeType::NewExpr:
            return evalNewExpr(std::static_pointer_cast<AST::NewExprType>(astNode), env);

        case AST::NodeType::BinaryExpr:
            return evalBinExpr(std::static_pointer_cast<AST::BinaryExprType>(astNode), env);
            
        case AST::NodeType::WhileStmt:
            return evalWhileStmt(std::static_pointer_cast<AST::WhileStmtType>(astNode), env);

        case AST::NodeType::Program:
            return evalProgram(std::static_pointer_cast<AST::ProgramType>(astNode), env, context);

        case AST::NodeType::NullLiteral:
            return std::make_shared<Values::NullVal>();

        case AST::NodeType::Identifier:
            return evalIdent(std::static_pointer_cast<AST::IdentifierType>(astNode), env);

        case AST::NodeType::MapLiteral:
            return evalObject(std::static_pointer_cast<AST::MapLiteralType>(astNode), env);

        case AST::NodeType::ArrayLiteral:
            return evalArray(std::static_pointer_cast<AST::ArrayLiteralType>(astNode), env);

        case AST::NodeType::CastExpr:
            return eval(std::static_pointer_cast<AST::CastExprType>(astNode)->left, env);

        case AST::NodeType::CallExpr:
            return evalCall(std::static_pointer_cast<AST::CallExprType>(astNode), env);

        case AST::NodeType::VarDeclaration:
            return evalVarDeclaration(std::static_pointer_cast<AST::VarDeclarationType>(astNode), env);

        case AST::NodeType::IfStmt:
            return evalIfStmt(std::static_pointer_cast<AST::IfStmtType>(astNode), env);

        case AST::NodeType::FunctionDeclaration:
            return evalFunctionDeclaration(std::static_pointer_cast<AST::FunctionDeclarationType>(astNode), env);

        case AST::NodeType::AssignmentExpr:
            return evalAssignment(std::static_pointer_cast<AST::AssignmentExprType>(astNode), env);

        case AST::NodeType::MemberExpr:
            return evalMemberExpr(std::static_pointer_cast<AST::MemberExprType>(astNode), env);

        case AST::NodeType::MemberAssignment:
            return evalMemberAssignment(std::static_pointer_cast<AST::MemberAssignmentType>(astNode), env);

        case AST::NodeType::ForStmt:
            return evalForStmt(std::static_pointer_cast<AST::ForStmtType>(astNode), env);

        case AST::NodeType::UnaryPostFix:
            return evalUnaryPostfix(std::static_pointer_cast<AST::UnaryPostFixType>(astNode), env);
        
        case AST::NodeType::UnaryPrefix:
            return evalUnaryPrefix(std::static_pointer_cast<AST::UnaryPrefixType>(astNode), env);

        case AST::NodeType::ArrowFunction:
            return evalArrowFunction(std::static_pointer_cast<AST::ArrowFunctionType>(astNode), env);

        case AST::NodeType::BreakStmt:
            throw Values::BreakSignal(CustomError("Did not expect break statement", "BreakError", astNode->token));
        
        case AST::NodeType::ContinueStmt:
            throw Values::ContinueSignal(CustomError("Did not expect continue statement", "ContinueError", astNode->token));

        case AST::NodeType::ImportStmt:
            return evalImportStmt(std::static_pointer_cast<AST::ImportStmtType>(astNode), env, context);

        case AST::NodeType::TernaryExpr:
            return evalTernaryExpr(std::static_pointer_cast<AST::TernaryExprType>(astNode), env);
        
        case AST::NodeType::TemplateCall:
            return evalTemplateCall(std::static_pointer_cast<AST::TemplateCallType>(astNode), env);
        
        case AST::NodeType::AwaitExpr:
            return evalAwaitExpr(std::static_pointer_cast<AST::AwaitExprType>(astNode), env);

        default:
            std::cerr << "Unexpected AST-node kind found: " << std::to_string(static_cast<int>(astNode->kind));
            exit(1);
    }
}