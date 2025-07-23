#include "compiler.hpp"

using namespace Probescript;

void Compiler::compile()
{
    for (auto& stmt : m_program->body)
    {
        gen(stmt);
    }
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

void Compiler::genIdent(std::shared_ptr<AST::IdentifierType> ident)
{
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

    builder->endFunction();
    builder->createStore(fn->name);
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

    if (assign->assigne->kind == AST::NodeType::Identifier)
    {
        builder->createAssign(std::static_pointer_cast<AST::IdentifierType>(assign->assigne)->symbol);
    }
    else
    {
        throw std::runtime_error("Unknown assignment type");
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