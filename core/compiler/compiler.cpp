#include "compiler.hpp"

using namespace Probescript;

extern std::unordered_map<std::string, VM::ValuePtr> g_valueGlobals;

void Compiler::compile()
{
    for (auto& stmt : m_program->body)
    {
        gen(stmt);
    }

    builder->createLoad("Main");
    builder->createCall(0);
}

void Compiler::gen(std::shared_ptr<AST::Stmt> node)
{
    if (!node) return;

    switch (node->kind)
    {
        case AST::NodeType::NumericLiteral:
            genNumber(std::static_pointer_cast<AST::NumericLiteralType>(node));
            break;        
        case AST::NodeType::Identifier:
            genIdent(std::static_pointer_cast<AST::IdentifierType>(node));
            break;
        case AST::NodeType::CallExpr:
            genCall(std::static_pointer_cast<AST::CallExprType>(node));
            break;
        case AST::NodeType::BinaryExpr:
            genBinExpr(std::static_pointer_cast<AST::BinaryExprType>(node));
            break;
        case AST::NodeType::FunctionDeclaration:
            genFunction(std::static_pointer_cast<AST::FunctionDeclarationType>(node));
            break;
        case AST::NodeType::VarDeclaration:
            genVarDecl(std::static_pointer_cast<AST::VarDeclarationType>(node));
            break;
        case AST::NodeType::IfStmt:
            genIf(std::static_pointer_cast<AST::IfStmtType>(node));
            break;
        case AST::NodeType::StringLiteral:
            genString(std::static_pointer_cast<AST::StringLiteralType>(node));
            break;
        case AST::NodeType::WhileStmt:
            genWhile(std::static_pointer_cast<AST::WhileStmtType>(node));
            break;
        case AST::NodeType::AssignmentExpr:
            genAssign(std::static_pointer_cast<AST::AssignmentExprType>(node));
            break;
        case AST::NodeType::MemberExpr:
            genMemberAccess(std::static_pointer_cast<AST::MemberExprType>(node));
            break;
        case AST::NodeType::ReturnStmt:
            genReturn(std::static_pointer_cast<AST::ReturnStmtType>(node));
            break;
        case AST::NodeType::UnaryPrefix:
            genUnaryPrefix(std::static_pointer_cast<AST::UnaryPrefixType>(node));
            break;
        case AST::NodeType::ProbeDeclaration:
            genProbe(std::static_pointer_cast<AST::ProbeDeclarationType>(node));
            break;

        default:
            throw std::runtime_error("Unknown AST node type");
    }
}

void Compiler::genNumber(std::shared_ptr<AST::NumericLiteralType> num)
{
    builder->createNumber(num->numValue);
}

void Compiler::genString(std::shared_ptr<AST::StringLiteralType> string)
{
    builder->createString(string->strValue);
}

void Compiler::genCall(std::shared_ptr<AST::CallExprType> call)
{
    gen(call->calee); // The calee is furthest down the stack
    for (auto& arg : call->args)
    {
        gen(arg);
    }

    builder->createCall(call->args.size());
}

void Compiler::genReturn(std::shared_ptr<AST::ReturnStmtType> returnStmt)
{
    gen(returnStmt->val),

    builder->createReturn();
}

void Compiler::genProbe(std::shared_ptr<AST::ProbeDeclarationType> probe)
{
    builder->startProbe();

    for (const auto stmt : probe->body)
    {
        gen(stmt);
    }

    builder->endProbe(probe->name);
    builder->createStore(probe->name);
}

void Compiler::genUnaryPrefix(std::shared_ptr<AST::UnaryPrefixType> unaryExpr)
{
    gen(unaryExpr->assigne);

    std::string op = unaryExpr->op;
    
    if (op == "!")
    {
        builder->createNegate();
        return;
    }

    if (op != "++" && op != "--")
    {
        throw std::runtime_error(CustomError("Unknown unary operator: " + op, "UnaryError", unaryExpr->token));
    }
    
    if (unaryExpr->assigne->kind == AST::NodeType::Identifier)
    {
        std::string ident = std::static_pointer_cast<AST::IdentifierType>(unaryExpr->assigne)->symbol;

        builder->createNumber(op == "++" ? 1 : -1);
        builder->createAdd();
        builder->createAssign(ident);
        return;
    }

    // TODO: Add support for member expressions
    throw std::runtime_error(CustomError("Unknown unary expression assigne", "UnaryError", unaryExpr->assigne->token));
}

void Compiler::genIdent(std::shared_ptr<AST::IdentifierType> ident)
{
    if (ident->symbol == "console")
    {
        builder->createLoadConsole();
        return;
    }

    if (g_valueGlobals.find(ident->symbol) != g_valueGlobals.end())
    {
        builder->createLoadGlobal(ident->symbol);
        return;
    }

    builder->createLoad(ident->symbol);
}

void Compiler::genBinExpr(std::shared_ptr<AST::BinaryExprType> expr)
{
    gen(expr->right);
    gen(expr->left);

    auto op = expr->op;

    if (op == "+")
    {
        builder->createAdd();
    }
    else if (op == "-")
    {
        builder->createSub();
    }
    else if (op == "*")
    {
        builder->createMul();
    }
    else if (op == "/")
    {
        builder->createDiv();
    }
    else if (op == "==")
    {
        builder->createEquals();
    }
    else if (op == ">")
    {
        builder->createGreaterThan();
    }
    else if (op == "<")
    {
        builder->createLessThan();
    }
    else
    {
        throw std::runtime_error("Unknown binary operator: " + op);
    }
}

void Compiler::genFunction(std::shared_ptr<AST::FunctionDeclarationType> fn)
{
    builder->startFunction();

    for (auto& stmt : fn->body)
    {
        gen(stmt);
    }

    std::vector<std::string> paramNames;
    std::transform(
        fn->parameters.begin(), fn->parameters.end(),
        std::back_inserter(paramNames),
        [](const std::shared_ptr<AST::VarDeclarationType>& param)
        {
            return param->identifier;
        }
    );

    builder->endFunction(paramNames);

    builder->createStore(fn->name);
}

void Compiler::genMemberAccess(std::shared_ptr<AST::MemberExprType> member)
{
    if (
        !member->computed
        && member->object->kind == AST::NodeType::Identifier
        && std::static_pointer_cast<AST::IdentifierType>(member->object)->symbol == "console"
    )
    {
        std::string property
            = std::static_pointer_cast<AST::IdentifierType>(member->property)->symbol;

        if (
            property != "println"
            && property != "print"
            && property != "prompt"
        )
        {
            throw std::runtime_error(CustomError("Console does not have property " + property, "MemberError", member->property->token));
        }

        builder->createLoadConsole(property);
        return;
    }

    gen(member->object);

    if (member->computed)
    {
        gen(member->property);
        builder->createMemberAccess();

        return;
    }

    builder->createMemberAccess(std::static_pointer_cast<AST::IdentifierType>(member->property)->symbol);
}

void Compiler::genVarDecl(std::shared_ptr<AST::VarDeclarationType> decl)
{
    gen(decl->value);

    builder->createStore(decl->identifier);
}

void Compiler::genIf(std::shared_ptr<AST::IfStmtType> ifStmt)
{
    gen(ifStmt->condition);

    builder->startIf();
    builder->startScope();

    for (const auto stmt : ifStmt->body)
    {
        gen(stmt);
    }

    if (ifStmt->hasElse) builder->createJump(0);
    size_t index = builder->getInstructionLength();

    builder->endScope();
    builder->endIf();

    if (ifStmt->hasElse)
    {
        builder->startScope();

        for (const auto stmt : ifStmt->elseStmt)
        {
            gen(stmt);
        }

        builder->endScope();
        builder->set(index, std::make_shared<VM::Instruction>(VM::Opcode::JUMP, builder->getInstructionLength()));
    }
}

void Compiler::genWhile(std::shared_ptr<AST::WhileStmtType> whileStmt)
{
    size_t loopStart = builder->getInstructionLength();

    gen(whileStmt->condition);

    size_t jumpIfFalseIndex = builder->getInstructionLength();
    builder->createJumpIfFalse(0);

    builder->startScope();
    for (const auto& stmt : whileStmt->body)
    {
        gen(stmt);
    }
    builder->endScope();

    builder->createJump(loopStart);
    builder->patchJumpIfFalse(jumpIfFalseIndex, builder->getInstructionLength());
}

void Compiler::genAssign(std::shared_ptr<AST::AssignmentExprType> assign)
{
    gen(assign->value);

    if (assign->assigne->kind != AST::NodeType::Identifier)
    {
        throw std::runtime_error(CustomError("Can only assign to identifiers", "AssignError", assign->assigne->token));
    }

    std::string assigne = std::static_pointer_cast<AST::IdentifierType>(assign->assigne)->symbol;

    if (assign->op == "=")
    {
        builder->createAssign(assigne);
    }
    else if (assign->op == "+=")
    {
        builder->createLoad(assigne);
        builder->createAdd();

        builder->createAssign(assigne);
    }
    else if (assign->op == "-=")
    {
        builder->createLoad(assigne);
        builder->createSub();

        builder->createAssign(assigne);
    }
    else if (assign->op == "*=")
    {
        builder->createLoad(assigne);
        builder->createMul();

        builder->createAssign(assigne);
    }
    else if (assign->op == "/=")
    {
        builder->createLoad(assigne);
        builder->createDiv();

        builder->createAssign(assigne);
    }
    else
    {
        throw std::runtime_error(CustomError("Unknown assignment operator", "AssignError", assign->token));
    }
}

std::vector<std::shared_ptr<VM::Instruction>> Compiler::getInstructions()
{
    return builder->getInstructions();
}

std::vector<VM::ValuePtr> Compiler::getConstants()
{
    return builder->getConstants();
}