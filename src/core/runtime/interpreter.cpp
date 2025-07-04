#include "runtime/interpreter.hpp"

Val evalArray(std::shared_ptr<ArrayLiteralType> expr, EnvPtr env) {
    std::vector<Val> items;

    for (std::shared_ptr<Expr> item : expr->items)
    {
        items.push_back(eval(item, env));
    }

    return makeVal<ArrayVal>(expr->token, items);
}

Val evalArrowFunction(std::shared_ptr<ArrowFunctionType> fn, EnvPtr env) {
    return makeVal<FunctionValue>(fn->token, "arrow", fn->params, env, fn->body);
}

Val evalTemplateCall(std::shared_ptr<TemplateCallType> call, EnvPtr env)
{
    Val caller = eval(call->caller, env);
    EnvPtr scope = std::make_shared<Env>(env);
    std::string name = "template";
    std::vector<std::shared_ptr<VarDeclarationType>> params;
    std::vector<std::shared_ptr<Stmt>> body;

    if (caller->type == ValueType::Function)
    {
        std::shared_ptr<FunctionValue> fn = std::static_pointer_cast<FunctionValue>(caller);
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
                    : std::make_shared<UndefinedVal>()),
                fn->templateparams[i]->token
            );
        }
    }

    return makeVal<FunctionValue>(call->token, name, params, scope, body);
}

Val evalAssignment(std::shared_ptr<AssignmentExprType> assignment, EnvPtr env)
{
    if (assignment->assigne->kind != NodeType::Identifier)
    {
        throw ThrowException(CustomError("Expected Identifier in assignment", "AssignmentError", assignment->token));
    }

    std::string varName = std::static_pointer_cast<IdentifierType>(assignment->assigne)->symbol;

    Val leftVal = eval(assignment->assigne, env);
    Val rightVal = eval(assignment->value, env);

    if (assignment->op == "=")
    {
        return env->assignVar(varName, rightVal, assignment->token);
    }

    Val result;

    if (assignment->op == "-=") result = leftVal->sub(rightVal);
    else if (assignment->op == "*=") result = leftVal->mul(rightVal);
    else if (assignment->op == "/=") result = leftVal->div(rightVal);
    else if (assignment->op == "+=") result = leftVal->add(rightVal);
    else {
        throw ThrowException(CustomError("Unsupported assignment operator: " + assignment->op, "AssignmentError", assignment->token));
    }

    return env->assignVar(varName, result, assignment->token);
}

Val evalUnaryPostfix(std::shared_ptr<UnaryPostFixType> expr, EnvPtr env)
{
    if (expr->assigne->kind == NodeType::Identifier)
    {
        std::string varName = std::static_pointer_cast<IdentifierType>(expr->assigne)->symbol;
        Val current = env->lookupVar(varName, expr->assigne->token);

        if (current->type != ValueType::Number)
        {
            throw ThrowException(CustomError("Postfix operators only supported on numbers", "OperatorError", current->token));
        }

        double value = std::static_pointer_cast<NumberVal>(current)->toNum();
        double newValue = value;

        if (expr->op == "++") newValue = value + 1;
        else if (expr->op == "--") newValue = value - 1;
        else
        {
            throw ThrowException(CustomError("Unknown postfix operator: " + expr->op, "OperatorError", expr->token));
        }

        env->assignVar(varName, makeVal<NumberVal>(expr->token, newValue), expr->token);

        return makeVal<NumberVal>(expr->token, value);
    } else if (expr->assigne->kind == NodeType::MemberExpr) {
        auto member = std::make_shared<MemberAssignmentType>(
            std::static_pointer_cast<MemberExprType>(expr->assigne)->object,
            std::static_pointer_cast<MemberExprType>(expr->assigne)->property,
            std::make_shared<NumericLiteralType>(1),
            std::static_pointer_cast<MemberExprType>(expr->assigne)->computed,
            expr->op
        );
        member->token = expr->token;

        return evalMemberAssignment(member, env);
    }

    return std::make_shared<UndefinedVal>();
}


Val evalUnaryPrefix(std::shared_ptr<UnaryPrefixType> expr, EnvPtr env) {
    Val val = eval(expr->assigne, env);

    if (expr->op == "!")
    {
        return makeVal<BooleanVal>(expr->token, !val->toBool());
    }

    return std::make_shared<UndefinedVal>();
}


std::unordered_set<std::string> booleanOperators = { "&&", "||", ">=", "<=", "<", ">", "!=", "==" };

Val evalBinExpr(std::shared_ptr<BinaryExprType> binop, EnvPtr env) {
    if (booleanOperators.count(binop->op))
    {
        return evalBooleanBinExpr(binop, env);
    }

    Val left = eval(binop->left, env);
    Val right = eval(binop->right, env);

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

Val evalBody(std::vector<std::shared_ptr<Stmt>> body, EnvPtr env, bool isLoop)
{
    for (std::shared_ptr<Stmt> stmt : body)
    {
        eval(stmt, env);
    }

    return std::make_shared<UndefinedVal>();
}

Val evalTernaryExpr(std::shared_ptr<TernaryExprType> expr, EnvPtr env)
{
    Val cond = eval(expr->cond, env);

    if (cond->toBool())
        return eval(expr->cons, env);
    else
        return eval(expr->alt, env);
}

Val evalBooleanBinExpr(std::shared_ptr<BinaryExprType> binop, EnvPtr env)
{
    Val left = eval(binop->left, env);
    Val right = eval(binop->right, env);

    const std::string& op = binop->op;

    if (op == "&&" || op == "||")
    {
        bool l = left->toBool();
        bool r = right->toBool();
        return makeVal<BooleanVal>(binop->token, ((op == "&&") ? (l && r) : (l || r)));
    }

    if (op == "==" || op == "!=")
    {
        bool result = *left == *right;
        return makeVal<BooleanVal>(binop->token, op == "==" ? result : !result);
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

        return makeVal<BooleanVal>(binop->token, result);
    }

    throw ThrowException(CustomError("Invalid binary boolean operator: " + op, "OperatorError", binop->token));
}

Val evalFunctionDeclaration(std::shared_ptr<FunctionDeclarationType> declaration, EnvPtr env, bool onlyValue)
{
    std::shared_ptr<FunctionValue> fn = makeVal<FunctionValue>(declaration->token, declaration->name, declaration->parameters, env, declaration->body, declaration->isAsync);
    fn->templateparams = declaration->templateparams;

    return onlyValue ? fn : env->declareVar(declaration->name, fn, declaration->token);
}

Val evalIdent(std::shared_ptr<IdentifierType> ident, EnvPtr env)
{
    Val value = env->lookupVar(ident->symbol, ident->token);
    return value;
}

Val evalIfStmt(std::shared_ptr<IfStmtType> stmt, EnvPtr baseEnv)
{
    Val condition = eval(stmt->condition, baseEnv);

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

    return std::make_shared<UndefinedVal>();
}

Val evalImportStmt(std::shared_ptr<ImportStmtType> importstmt, EnvPtr envptr, std::shared_ptr<Context> config)
{
    std::string modulename = importstmt->name;

    std::unordered_map<std::string, Val> stdlib;

    for (const auto& [key, pair] : g_stdlib)
        stdlib[key] = pair.first;
        
    if (stdlib.find(modulename) != stdlib.end())
    {
        if (importstmt->hasMember)
        {
            std::shared_ptr<Expr> member = importstmt->module;
            EnvPtr modEnv = std::make_shared<Env>();
            modEnv->declareVar(modulename, stdlib[modulename], member->token);
            envptr->declareVar(importstmt->customIdent ? importstmt->ident : std::static_pointer_cast<MemberExprType>(importstmt->module)->lastProp, eval(member, modEnv), member->token);
        }
        else envptr->declareVar(importstmt->customIdent ? importstmt->ident : modulename, stdlib[modulename], importstmt->token);
        return std::make_shared<UndefinedVal>();
    }

    if (config->modules.find(modulename) == config->modules.end())
    {
        throw ThrowException(CustomError("Cannot find module " + modulename, "ImportError", importstmt->token));
    }

    fs::path filepath = config->modules[modulename];

    std::ifstream stream(filepath);

    std::string file((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

    Parser parser;

    std::shared_ptr<ProgramType> program = parser.parse(file);

    std::shared_ptr<Context> conf = std::make_shared<Context>(RuntimeType::Exports);

    conf->modules = config->modules;

    Val evaluated = eval(program, std::make_shared<Env>(), conf);

    std::shared_ptr<ObjectVal> moduleObj = makeVal<ObjectVal>(importstmt->token, evaluated->exports);

    if (importstmt->hasMember)
    {
        std::shared_ptr<Expr> member = importstmt->module;
        EnvPtr modEnv = std::make_shared<Env>();
        modEnv->declareVar(modulename, moduleObj, member->token);
        envptr->declareVar(importstmt->customIdent ? importstmt->ident : std::static_pointer_cast<MemberExprType>(importstmt->module)->lastProp, eval(member, modEnv), member->token);
    }
    else envptr->declareVar(importstmt->customIdent ? importstmt->ident : modulename, moduleObj, importstmt->token);

    return std::make_shared<UndefinedVal>();
}

Val evalMemberAssignment(std::shared_ptr<MemberAssignmentType> expr, EnvPtr env)
{
    Val obj = eval(expr->object, env);
    Val value = eval(expr->newvalue, env);

    std::string key;

    if (expr->computed)
    {
        Val propValue = eval(expr->property, env);

        if (propValue->type == ValueType::Number)
        {
            int index = propValue->toNum();

            if (obj->type == ValueType::Array)
            {
                std::shared_ptr<ArrayVal> array = std::static_pointer_cast<ArrayVal>(obj);

                if (index >= array->items.size())
                {
                    array->items.resize(index + 1, std::make_shared<UndefinedVal>());
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

        if (propValue->type != ValueType::String)
        {
            throw ThrowException(CustomError("Computed property must evaluate to a string or number", "MemberError", expr->token));
        }

        key = std::static_pointer_cast<StringVal>(propValue)->string;
    }
    else
    {
        std::shared_ptr<IdentifierType> ident = std::static_pointer_cast<IdentifierType>(expr->property);
        key = ident->symbol;
    }

    if (obj->type == ValueType::Object)
    {
        std::shared_ptr<ObjectVal> objectVal = std::static_pointer_cast<ObjectVal>(obj);
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

Val evalMemberExpr(std::shared_ptr<MemberExprType> expr, EnvPtr env)
{
    Val obj = eval(expr->object, env);
    
    if (obj->type != ValueType::Array || !expr->computed)
    {
        std::string key;

        if (expr->computed)
        {
            Val propValue = eval(expr->property, env);
    
            if (propValue->type != ValueType::String)
            {
                throw ThrowException(CustomError("Computed property must evaluate to a string", "TypeError", expr->token));
            }
    
            key = std::static_pointer_cast<StringVal>(propValue)->string;
        }
        else
        {
            std::shared_ptr<IdentifierType> ident = std::static_pointer_cast<IdentifierType>(expr->property);
            key = ident->symbol;
        }

        Val object = obj;

        if (object->properties.count(key) == 0)
        {
            return std::make_shared<UndefinedVal>();
        }

        return object->properties[key];
    }
    else
    {
        Val indexval = eval(expr->property, env);

        if (indexval->type != ValueType::Number)
        {
            throw ThrowException(CustomError("Array index must evaluate to a number", "TypeError", expr->token));
        }

        std::shared_ptr<NumberVal> index = std::static_pointer_cast<NumberVal>(indexval);

        std::shared_ptr<ArrayVal> array = std::static_pointer_cast<ArrayVal>(obj);

        int idx = index->number;

        if (idx < 0 || idx >= static_cast<int>(array->items.size()))
        {
            return std::make_shared<UndefinedVal>();
        }

        return array->items[idx];
    }
}

Val evalObject(std::shared_ptr<MapLiteralType> obj, EnvPtr env)
{
    std::shared_ptr<ObjectVal> object = std::make_shared<ObjectVal>();
    for (std::shared_ptr<PropertyLiteralType> property : obj->properties)
    {
        Val runtimeval = (property->val == nullptr) ? env->lookupVar(property->key, property->token) : eval(property->val, env);
        object->properties[property->key] = runtimeval;
    }

    return object;
}

Val evalVarDeclaration(std::shared_ptr<VarDeclarationType> decl, EnvPtr env, bool constant)
{
    Val value = decl->value != nullptr ? eval(decl->value, env) : std::make_shared<UndefinedVal>();
    return env->declareVar(decl->identifier, value, decl->token);
}

Val evalThrowStmt(std::shared_ptr<ThrowStmtType> stmt, EnvPtr env)
{
    throw ThrowException(eval(stmt->err, env)->toString());
}

Val evalTryStmt(std::shared_ptr<TryStmtType> stmt, EnvPtr env)
{
    std::shared_ptr<FunctionValue> fn = makeVal<FunctionValue>(stmt->catchHandler->token, "catch", stmt->catchHandler->parameters, env, stmt->catchHandler->body);
    EnvPtr scope = std::make_shared<Env>(env);

    try
    {
        evalBody(stmt->body, scope);
    }
    catch (const ThrowException& e)
    {
        evalCallWithFnVal(fn, { std::make_shared<StringVal>(e.what()) }, scope);
    }

    return std::make_shared<UndefinedVal>();
}

Val eval(std::shared_ptr<Stmt> astNode, EnvPtr env, std::shared_ptr<Context> config)
{
    switch (astNode->kind)
    {
        case NodeType::NumericLiteral:
        {
            std::shared_ptr<NumericLiteralType> num = std::static_pointer_cast<NumericLiteralType>(astNode);
            return makeVal<NumberVal>(astNode->token, num->value());
        }

        case NodeType::StringLiteral:
        {
            std::shared_ptr<StringLiteralType> str = std::static_pointer_cast<StringLiteralType>(astNode);
            return makeVal<StringVal>(astNode->token, str->value());
        }

        case NodeType::UndefinedLiteral:
        {
            return std::make_shared<UndefinedVal>();
        }

        case NodeType::BoolLiteral:
            return makeVal<BooleanVal>(astNode->token, std::static_pointer_cast<BoolLiteralType>(astNode)->value);

        case NodeType::TryStmt:
            return evalTryStmt(std::static_pointer_cast<TryStmtType>(astNode), env);

        case NodeType::ThrowStmt:
            return evalThrowStmt(std::static_pointer_cast<ThrowStmtType>(astNode), env);

        case NodeType::ReturnStmt:
        {
            Val value = eval(std::static_pointer_cast<ReturnStmtType>(astNode)->val, env);
            throw ReturnSignal(value, CustomError("Did not expect return statement", "ReturnError", astNode->token));
        }
        
        case NodeType::ClassDefinition:
            return evalClassDefinition(std::static_pointer_cast<ClassDefinitionType>(astNode), env);

        case NodeType::ProbeDeclaration:
            return evalProbeDeclaration(std::static_pointer_cast<ProbeDeclarationType>(astNode), env);
            
        case NodeType::NewExpr:
            return evalNewExpr(std::static_pointer_cast<NewExprType>(astNode), env);

        case NodeType::BinaryExpr:
            return evalBinExpr(std::static_pointer_cast<BinaryExprType>(astNode), env);
            
        case NodeType::WhileStmt:
            return evalWhileStmt(std::static_pointer_cast<WhileStmtType>(astNode), env);

        case NodeType::Program:
            return evalProgram(std::static_pointer_cast<ProgramType>(astNode), env, config);

        case NodeType::NullLiteral:
            return std::make_shared<NullVal>();

        case NodeType::Identifier:
            return evalIdent(std::static_pointer_cast<IdentifierType>(astNode), env);

        case NodeType::MapLiteral:
            return evalObject(std::static_pointer_cast<MapLiteralType>(astNode), env);

        case NodeType::ArrayLiteral:
            return evalArray(std::static_pointer_cast<ArrayLiteralType>(astNode), env);

        case NodeType::CastExpr:
            return eval(std::static_pointer_cast<CastExprType>(astNode)->left, env);

        case NodeType::CallExpr:
            return evalCall(std::static_pointer_cast<CallExprType>(astNode), env);

        case NodeType::VarDeclaration:
            return evalVarDeclaration(std::static_pointer_cast<VarDeclarationType>(astNode), env);

        case NodeType::IfStmt:
            return evalIfStmt(std::static_pointer_cast<IfStmtType>(astNode), env);

        case NodeType::FunctionDeclaration:
            return evalFunctionDeclaration(std::static_pointer_cast<FunctionDeclarationType>(astNode), env);

        case NodeType::AssignmentExpr:
            return evalAssignment(std::static_pointer_cast<AssignmentExprType>(astNode), env);

        case NodeType::MemberExpr:
            return evalMemberExpr(std::static_pointer_cast<MemberExprType>(astNode), env);

        case NodeType::MemberAssignment:
            return evalMemberAssignment(std::static_pointer_cast<MemberAssignmentType>(astNode), env);

        case NodeType::ForStmt:
            return evalForStmt(std::static_pointer_cast<ForStmtType>(astNode), env);

        case NodeType::UnaryPostFix:
            return evalUnaryPostfix(std::static_pointer_cast<UnaryPostFixType>(astNode), env);
        
        case NodeType::UnaryPrefix:
            return evalUnaryPrefix(std::static_pointer_cast<UnaryPrefixType>(astNode), env);

        case NodeType::ArrowFunction:
            return evalArrowFunction(std::static_pointer_cast<ArrowFunctionType>(astNode), env);

        case NodeType::BreakStmt:
            throw BreakSignal(CustomError("Did not expect break statement", "BreakError", astNode->token));
        
        case NodeType::ContinueStmt:
            throw ContinueSignal(CustomError("Did not expect continue statement", "ContinueError", astNode->token));

        case NodeType::ImportStmt:
            return evalImportStmt(std::static_pointer_cast<ImportStmtType>(astNode), env, config);

        case NodeType::TernaryExpr:
            return evalTernaryExpr(std::static_pointer_cast<TernaryExprType>(astNode), env);
        
        case NodeType::TemplateCall:
            return evalTemplateCall(std::static_pointer_cast<TemplateCallType>(astNode), env);
        
        case NodeType::AwaitExpr:
            return evalAwaitExpr(std::static_pointer_cast<AwaitExprType>(astNode), env);

        default:
            std::cerr << "Unexpected AST-node kind found: " << std::to_string(static_cast<int>(astNode->kind));
            exit(1);
    }
}