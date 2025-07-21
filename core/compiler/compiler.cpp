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

        default:
            throw std::runtime_error("Unknown AST node type: " + node->kind);
    }
}

void Compiler::genNumber(std::shared_ptr<AST::NumericLiteralType> num)
{
    builder->createNumber(num->numValue);
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
    gen(expr->left);
    gen(expr->right);

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

std::vector<std::shared_ptr<VM::Instruction>> Compiler::getInstructions()
{
    return builder->getInstructions();
}

std::vector<VM::ValuePtr> Compiler::getConstants()
{
    return builder->getConstants();
}