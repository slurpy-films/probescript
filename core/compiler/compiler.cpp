#include "compiler.hpp"

#include "errors.hpp"

#include "frontend/ast.hpp"
#include "frontend/parser.hpp"
#include <memory>
#include <string>

using namespace Probescript;

extern std::unordered_map<std::string, VM::ValuePtr> g_valueGlobals;
extern std::unordered_map<std::string, VM::ValuePtr> g_valueStdlib;

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
        case AST::NodeType::NullLiteral:
            builder->createNull();
            break;
        case AST::NodeType::UndefinedLiteral:
            builder->createNull(); // After 0.0.6, undefined is deprecated in favor of null
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
        case AST::NodeType::BoolLiteral:
            genBoolean(std::static_pointer_cast<AST::BoolLiteralType>(node));
            break;
        case AST::NodeType::ForStmt:
            genFor(std::static_pointer_cast<AST::ForStmtType>(node));
            break;
        case AST::NodeType::UnaryPostFix:
            genUnaryPostfix(std::static_pointer_cast<AST::UnaryPostFixType>(node));
            break;
        case AST::NodeType::ArrowFunction:
            genArrowFn(std::static_pointer_cast<AST::ArrowFunctionType>(node));
            break;
        case AST::NodeType::MemberAssignment:
            genMemberAssign(std::static_pointer_cast<AST::MemberAssignmentType>(node));
            break;
        case AST::NodeType::MapLiteral:
            genMapLiteral(std::static_pointer_cast<AST::MapLiteralType>(node));
            break;
        case AST::NodeType::NewExpr:
            genNewExpr(std::static_pointer_cast<AST::NewExprType>(node));
            break;
        case AST::NodeType::ImportStmt:
            genImport(std::static_pointer_cast<AST::ImportStmtType>(node));
            break;
        case AST::NodeType::BreakStmt:
            genBreak(std::static_pointer_cast<AST::BreakStmtType>(node));
            break;
        case AST::NodeType::ContinueStmt:
            genContinue(std::static_pointer_cast<AST::ContinueStmtType>(node));
            break;
        case AST::NodeType::ClassDefinition:
            genClass(std::static_pointer_cast<AST::ClassDefinitionType>(node));
            break;
        case AST::NodeType::TemplateCall:
            gen(std::static_pointer_cast<AST::TemplateCallType>(node)->caller);
            break;
        case AST::NodeType::CastExpr:
            gen(std::static_pointer_cast<AST::CastExprType>(node)->left);
            break;
        case AST::NodeType::ExportStmt:
            genExport(std::static_pointer_cast<AST::ExportStmtType>(node));
            break;
        case AST::NodeType::ArrayLiteral:
            genArrayLiteral(std::static_pointer_cast<AST::ArrayLiteralType>(node));
            break;
        case AST::NodeType::AwaitExpr:
            genAwait(std::static_pointer_cast<AST::AwaitExprType>(node));
            break;
        case AST::NodeType::ThrowStmt:
            genThrow(std::static_pointer_cast<AST::ThrowStmtType>(node));
            break;
        case AST::NodeType::TryStmt:
            genTry(std::static_pointer_cast<AST::TryStmtType>(node));
            break;
            
        case AST::NodeType::Empty:
            break;

        default:
            throw std::runtime_error("Unknown AST node type: " + std::to_string((int)node->kind));
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

void Compiler::genThrow(std::shared_ptr<AST::ThrowStmtType> stmt)
{
    gen(stmt->err);
    builder->createThrow();
}

void Compiler::genCall(std::shared_ptr<AST::CallExprType> call)
{
    gen(call->calee); // The calee is furthest down the stack

    for (const auto& arg : call->args)
    {
        gen(arg);
    }

    builder->createCall(call->args.size());
}

void Compiler::genNewExpr(std::shared_ptr<AST::NewExprType> expr)
{
    gen(expr->constructor);

    for (const auto& arg : expr->args)
    {
        gen(arg);
    }

    builder->createNew(expr->args.size());
}

void Compiler::genBoolean(std::shared_ptr<AST::BoolLiteralType> boolean)
{
    builder->createBoolLiteral(boolean->value);
}

void Compiler::genReturn(std::shared_ptr<AST::ReturnStmtType> returnStmt)
{
    gen(returnStmt->val);

    builder->createReturn();
}

void Compiler::genMemberAssign(std::shared_ptr<AST::MemberAssignmentType> assign)
{
    // The object has to be at the bottom of the stack
    gen(assign->object);
    gen(assign->newvalue);

    std::string property;
    if (assign->computed)
    {
        gen(assign->property);
        property = ""; // If the VM detects an empty property, it will assume the member expression is computed
    }
    else
    {
        property = std::static_pointer_cast<AST::IdentifierType>(assign->property)->symbol;
    }
    
    if (assign->op == "=")
    {
        builder->createMemberAssign(property);
    }
    else
    {
        throw std::runtime_error(CustomError("Unknown assignment operator", "AssignError", assign->token));
    }
}

void Compiler::genImport(std::shared_ptr<AST::ImportStmtType> stmt)
{
    if (g_valueStdlib.find(stmt->name) == g_valueStdlib.end())
    {
        auto modules = m_context->modules;

        if (modules.find(stmt->name) != modules.end())
        {
            fs::path path(modules[stmt->name]);
            std::ifstream stream(path);

            std::string file((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

            auto context = std::make_shared<Context>();
            context->file = file;
            context->filename = path.string();
            context->modules = modules;
            
            Parser parser;
            std::shared_ptr<AST::ProgramType> program = parser.parse(file, context);
            
            builder->startModule();
            for (const auto& stmt : program->body)
            {
                gen(stmt);
            } 
            builder->endModule();
        }
        else
        {
            throw std::runtime_error(
                CustomError("Module '" + stmt->name + "' not found", "ImportError", stmt->token));
        }
    }
    else
    {
        builder->createLoadStdlib(stmt->name);
    }

    std::string identifier = stmt->name;

    // If the import statement has a member expression we need to evaluate it
    if (stmt->hasMember)
    {
        if (stmt->module->kind != AST::NodeType::MemberExpr)
        {
            throw std::runtime_error(CustomError("You can only use member expressions to import directly", "ImportError", stmt->token));
        }

        builder->createStore(stmt->name);

        auto memberExpr = std::static_pointer_cast<AST::MemberExprType>(stmt->module);
        genMemberAccess(memberExpr);
        identifier = memberExpr->lastProp;
    }

    builder->createStore(stmt->customIdent ? stmt->ident : identifier);
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

void Compiler::genTry(std::shared_ptr<AST::TryStmtType> tryStmt)
{
    genFunction(tryStmt->catchHandler, true); // Only get the value, we do not want to store this function

    builder->startCatch();

    for (const auto stmt : tryStmt->body)
    {
        gen(stmt);
    }

    builder->endCatch();
}

void Compiler::genExport(std::shared_ptr<AST::ExportStmtType> exportStmt)
{
    gen(exportStmt->exporting);

    switch (exportStmt->exporting->kind)
    {
        case AST::NodeType::FunctionDeclaration:
        {
            builder->createExport(std::static_pointer_cast<AST::FunctionDeclarationType>(exportStmt->exporting)->name);
            break;
        }
        case AST::NodeType::VarDeclaration:
        {
           builder->createExport(std::static_pointer_cast<AST::VarDeclarationType>(exportStmt->exporting)->identifier);
           break; 
        }
        case AST::NodeType::ClassDefinition:
        {
            builder->createExport(std::static_pointer_cast<AST::ClassDefinitionType>(exportStmt->exporting)->name);
            break;
        }
        case AST::NodeType::ProbeDeclaration:
        {
            builder->createExport(std::static_pointer_cast<AST::ProbeDeclarationType>(exportStmt->exporting)->name);
            break;
        }
        case AST::NodeType::Identifier:
        {
            builder->createExport(std::static_pointer_cast<AST::IdentifierType>(exportStmt->exporting)->symbol);
            break;
        }

        default:
        {
            throw std::runtime_error(CustomError("Only variables, classes, identifiers, probes, and functions can be exported", "ExportError", exportStmt->exporting->token));
        }
    }
}

void Compiler::genArrayLiteral(std::shared_ptr<AST::ArrayLiteralType> array)
{
    std::string name = "__tempArray__" + std::to_string(builder->getVarCounter());
    builder->createArray();
    builder->createStore(name);

    builder->createPop(); // Pop the result of STORE

    for (const auto& item : array->items)
    {
        builder->createLoad(name);
        gen(item);
        builder->pushToArray();
    }

    builder->createLoad(name);
}

void Compiler::genMapLiteral(std::shared_ptr<AST::MapLiteralType> map)
{
    std::string tempMapName = "__tempMap__" + std::to_string(builder->getVarCounter());
    builder->createObject();
    builder->createStore(tempMapName);

    builder->createPop(); // Pop the result of STORE

    for (const auto& prop : map->properties)
    {
        builder->createLoad(tempMapName);
        gen(prop->val);
        builder->createMemberAssign(prop->key);
        builder->createPop(); // Pop the member assignment result as we don't need it
    }

    builder->createLoad(tempMapName);
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

    if (unaryExpr->assigne->kind == AST::NodeType::MemberExpr)
    {
        std::string tempName = "__tempUnary__" + std::to_string(builder->getVarCounter());
        builder->createStore(tempName);

        builder->createPop(); // Pop the result of STORE

        auto member = std::make_shared<AST::MemberAssignmentType>(
            std::static_pointer_cast<AST::MemberExprType>(unaryExpr->assigne)->object,
            std::static_pointer_cast<AST::MemberExprType>(unaryExpr->assigne)->property,
            std::make_shared<AST::BinaryExprType>(
                std::make_shared<AST::IdentifierType>(tempName),
                std::make_shared<AST::NumericLiteralType>(1),
                unaryExpr->op == "++" ? "+" : "-"
            ),
            std::static_pointer_cast<AST::MemberExprType>(unaryExpr->assigne)->computed,
            "="
        );

        genMemberAssign(member);

        return;
    }

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
    else if (op == "&&")
    {
        builder->createAnd();
    }
    else if (op == "||")
    {
        builder->createOr();
    }
    else if (op == "<=")
    {
        builder->createLessThanOrEqualTo();
    }
    else if (op == ">=")
    {
        builder->createGreaterThanOrEqualTo();
    }
    else if (op == "!=")
    {
        builder->createNotEquals();
    }
    else
    {
        throw std::runtime_error("Unknown binary operator: " + op);
    }
}

void Compiler::genFunction(std::shared_ptr<AST::FunctionDeclarationType> fn, bool onlyValue)
{
    builder->startFunction();

    std::vector<std::string> paramNames;
    for (const auto& param : fn->parameters)
    {
        paramNames.push_back(param->identifier);
        if (param->value && param->value->kind != AST::NodeType::UndefinedLiteral)
        {
            gen(param->value);
            builder->createDefaultParam(param->identifier);
        }
    }

    for (auto& stmt : fn->body)
    {
        gen(stmt);
    }


    builder->endFunction(paramNames, fn->name);

    if (fn->isAsync)
    {
        builder->createAsync();
    }

    if (!onlyValue)
    {
        builder->createStore(fn->name);
    }
}

void Compiler::genAwait(std::shared_ptr<AST::AwaitExprType> expr)
{
    gen(expr->caller);

    builder->createAwait();
}

void Compiler::genArrowFn(std::shared_ptr<AST::ArrowFunctionType> fn)
{
    builder->startFunction();

    for (auto& stmt : fn->body)
    {
        gen(stmt);
    }

    std::vector<std::string> paramNames;
    std::transform(
        fn->params.begin(), fn->params.end(),
        std::back_inserter(paramNames),
        [](const std::shared_ptr<AST::VarDeclarationType>& param)
        {
            return param->identifier;
        }
    );

    builder->endFunction(paramNames, "_");
}

void Compiler::genClass(std::shared_ptr<AST::ClassDefinitionType> cls)
{
    if (cls->doesExtend)
    {
        gen(cls->extends);
    }

    builder->startClass();

    for (const auto& stmt : cls->body)
    {
        gen(stmt);
    }

    builder->endClass(cls->doesExtend);

    builder->createStore(cls->name);
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

    size_t index = builder->getInstructionLength();
    if (ifStmt->hasElse) builder->createJump(0);

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
        builder->set(index, std::make_shared<VM::Instruction>(VM::Opcode::JUMP, builder->getInstructionLength(), true));
    }
}

void Compiler::enterLoop()
{
    m_breakPatchesStack.push_back(std::vector<size_t>());
    m_continuePatchesStack.push_back(std::vector<size_t>());
}

void Compiler::exitLoop(size_t continueTarget, size_t breakTarget)
{
    if (m_breakPatchesStack.empty() || m_continuePatchesStack.empty())
    {
        throw std::runtime_error("Internal compiler error\n");
    }

    for (size_t patchIndex : m_breakPatchesStack.back())
    {
        builder->set(patchIndex, std::make_shared<VM::Instruction>(VM::Opcode::JUMP, breakTarget, true));
    }

    for (size_t patchIndex : m_continuePatchesStack.back())
    {
        builder->set(patchIndex, std::make_shared<VM::Instruction>(VM::Opcode::JUMP, continueTarget, true));
    }

    m_breakPatchesStack.pop_back();
    m_continuePatchesStack.pop_back();
}

bool Compiler::isInLoop() const
{
    return !m_breakPatchesStack.empty();
}

void Compiler::genBreak(std::shared_ptr<AST::BreakStmtType> stmt)
{
    if (!isInLoop())
    {
        throw std::runtime_error(CustomError("'break' outside loop", "BreakError", stmt->token));
    }

    size_t patchIndex = builder->getInstructionLength();
    builder->createJump(0); // This will be patched later by the exitLoop method

    m_breakPatchesStack.back().push_back(patchIndex);
}

void Compiler::genContinue(std::shared_ptr<AST::ContinueStmtType> stmt)
{
    if (!isInLoop())
    {
        throw std::runtime_error(CustomError("'continue' outside loop", "ContinueError", stmt->token));
    }

    size_t patchIndex = builder->getInstructionLength();
    builder->createJump(0); // This will be patched later by the exitLoop method

    m_continuePatchesStack.back().push_back(patchIndex);
}

void Compiler::genWhile(std::shared_ptr<AST::WhileStmtType> whileStmt)
{
    enterLoop();
    
    size_t conditionStart = builder->getInstructionLength();
    gen(whileStmt->condition);
    
    size_t jumpIfFalseIndex = builder->getInstructionLength();
    builder->createJumpIfFalse(0);
    
    builder->startScope();
    for (const auto& stmt : whileStmt->body)
    {
        gen(stmt);
    }
    builder->endScope();
    
    builder->createJump(conditionStart);
    size_t loopEnd = builder->getInstructionLength();
    
    builder->patchJumpIfFalse(jumpIfFalseIndex, loopEnd);
    
    exitLoop(conditionStart, loopEnd);
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

void Compiler::genFor(std::shared_ptr<AST::ForStmtType> forStmt)
{
    enterLoop(); // Start tracking break and continue statements

    // Parent scope - holds the variables declared in the for-loop declarations
    builder->startScope();

    for (const auto& stmt : forStmt->declarations)
    {
        gen(stmt);
    }
    
    size_t loopStart = builder->getInstructionLength();
    std::vector<size_t> jumpIndexes;

    for (const auto& cond : forStmt->conditions)
    {
        gen(cond);
        jumpIndexes.push_back(builder->getInstructionLength());
        builder->createJumpIfFalse(0);
    }

    builder->startScope();

    for (const auto& stmt : forStmt->body)
    {
        gen(stmt);
    }

    size_t continueTarget = builder->getInstructionLength();

    for (const auto& update : forStmt->updates)
    {
        gen(update);
    }

    builder->endScope();
    builder->createJump(loopStart);

    size_t loopEnd = builder->getInstructionLength();

    for (const size_t& index : jumpIndexes)
    {
        builder->patchJumpIfFalse(index, builder->getInstructionLength());
    }

    builder->endScope();

    exitLoop(continueTarget, loopEnd);
}

void Compiler::genUnaryPostfix(std::shared_ptr<AST::UnaryPostFixType> unaryExpr)
{
    if (unaryExpr->op != "++" && unaryExpr->op != "--")
    {
        throw std::runtime_error("Unknown unary operator\n");
    }

    gen(unaryExpr->assigne);
    builder->createNumber(unaryExpr->op == "++" ? 1 : -1);
    builder->createAdd();

    if (unaryExpr->assigne->kind == AST::NodeType::Identifier)
    {
        std::string ident = std::static_pointer_cast<AST::IdentifierType>(unaryExpr->assigne)->symbol;

        builder->createAssign(ident);
        builder->createPop(); // Pop the result of the assignment as it will not be used

        builder->createNumber(unaryExpr->op == "++" ? 1 : -1);
        builder->createLoad(ident);
        builder->createSub();
        return;
    }

    if (unaryExpr->assigne->kind == AST::NodeType::MemberExpr)
    {
        std::string tempName = "__tempUnary__" + std::to_string(builder->getVarCounter());
        builder->createStore(tempName);

        builder->createPop(); // Pop the result of STORE

        auto member = std::make_shared<AST::MemberAssignmentType>(
            std::static_pointer_cast<AST::MemberExprType>(unaryExpr->assigne)->object,
            std::static_pointer_cast<AST::MemberExprType>(unaryExpr->assigne)->property,
            std::make_shared<AST::IdentifierType>(tempName),
            std::static_pointer_cast<AST::MemberExprType>(unaryExpr->assigne)->computed,
            "="
        );

        genMemberAssign(member);
        builder->createNumber(1);
        builder->createSwitchTop(); // Flip the top of the stack so we don't get 1 - member
        
        // Set it back to what it was first
        if (unaryExpr->op == "++")
        {
            builder->createSub();
        }
        else
        {
            builder->createAdd();
        }

        return;
    }

    throw std::runtime_error(CustomError("Unknown unary expression assigne", "UnaryError", unaryExpr->assigne->token));
}

std::vector<std::shared_ptr<VM::Instruction>> Compiler::getInstructions()
{
    return builder->getInstructions();
}

std::vector<VM::ValuePtr> Compiler::getConstants()
{
    return builder->getConstants();
}
