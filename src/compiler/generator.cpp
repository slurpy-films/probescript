#include "compiler.hpp"

using namespace probescript;

void Compiler::genProgram(std::shared_ptr<ProgramType> program)
{
    auto scope = std::make_shared<Scope>(module.get(), builder.get());
    for (auto stmt : program->body)
    {
        gen(stmt, scope);
    }
}

llvm::Value* Compiler::gen(std::shared_ptr<Stmt> node, std::shared_ptr<Scope> scope)
{
    if (!node)
        throw std::runtime_error("gen() called with nullptr");
    
    switch (node->kind)
    {
        case NodeType::NumericLiteral:
            return genNum(std::static_pointer_cast<NumericLiteralType>(node), scope);
        case NodeType::ReturnStmt:
            return genReturn(std::static_pointer_cast<ReturnStmtType>(node), scope);
        case NodeType::VarDeclaration:
            return genVarDecl(std::static_pointer_cast<VarDeclarationType>(node), scope);
        case NodeType::FunctionDeclaration:
            return genFunction(std::static_pointer_cast<FunctionDeclarationType>(node), scope);
        case NodeType::Identifier:
            return genIdent(std::static_pointer_cast<IdentifierType>(node), scope);
        case NodeType::CallExpr:
            return genCall(std::static_pointer_cast<CallExprType>(node), scope);
        default:
            throw std::runtime_error("Unknown node type in gen()");
    }
}

llvm::Value* Compiler::genReturn(std::shared_ptr<ReturnStmtType> stmt, std::shared_ptr<Scope> scope)
{
    if (!m_retptr)
    {
        throw std::runtime_error(CustomError("Did not expect return statement", "ReturnError", stmt->token));
    }

    llvm::Value* val = gen(stmt->val, scope);

    llvm::Value* tagGEP = builder->CreateStructGEP(valuetype, m_retptr, 0, "ret_tag");
    llvm::Value* tagVal = builder->CreateExtractValue(val, {0});
    builder->CreateStore(tagVal, tagGEP);

    llvm::Value* payloadGEP = builder->CreateStructGEP(valuetype, m_retptr, 1, "ret_payload");
    llvm::Value* payloadVal = builder->CreateExtractValue(val, {1});
    builder->CreateStore(payloadVal, payloadGEP);

    return builder->CreateRetVoid();
}

llvm::Value* Compiler::genProbe(std::shared_ptr<ProbeDeclarationType> prb, std::shared_ptr<Scope> scope)
{
    auto fn = createFunction(prb->name, llvm::FunctionType::get(valuetype, false));
    
    for (auto stmt : prb->body)
    {
        gen(stmt, scope);
    }

    return fn;
}

llvm::Value* Compiler::genFunction(std::shared_ptr<FunctionDeclarationType> fn, std::shared_ptr<Scope> scope)
{
    std::vector<llvm::Type*> parameters;
    parameters.push_back(llvm::PointerType::getUnqual(valuetype));
    parameters.resize(fn->parameters.size() + 1, valuetype);

    auto function = createFunction(fn->name, llvm::FunctionType::get(builder->getVoidTy(), llvm::ArrayRef(parameters), false));

    auto argIt = function->arg_begin();

    llvm::Argument* retptr = &(*argIt);
    retptr->setName("__sret");

    m_retptr = retptr;

    ++argIt;
    int idx = 0;

    auto fnScope = std::make_shared<Scope>(module.get(), builder.get(), scope);
    for (; argIt != function->arg_end(); ++argIt)
    {
        argIt->setName(fn->parameters[idx]->identifier);
        llvm::AllocaInst* alloc = builder->CreateAlloca(valuetype, nullptr, argIt->getName());

        builder->CreateStore(&(*argIt), alloc);

        fnScope->declareVar(fn->parameters[idx]->identifier, alloc);

        idx++;
    }

    for (auto stmt : fn->body)
    {
        gen(stmt, fnScope);
    }

    if (!builder->GetInsertBlock()->getTerminator())
    {
        llvm::Value* tagGEP = builder->CreateStructGEP(valuetype, m_retptr, 0, "sret_tag");
        builder->CreateStore(llvm::ConstantInt::get(builder->getInt32Ty(), 0), tagGEP);

        llvm::Value* payloadGEP = builder->CreateStructGEP(valuetype, m_retptr, 1, "sret_payload");
        llvm::Value* zeroDouble = llvm::ConstantFP::get(builder->getDoubleTy(), 0.0);

        llvm::Value* castedPtr = builder->CreateBitCast(payloadGEP, builder->getDoubleTy()->getPointerTo());
        builder->CreateStore(zeroDouble, castedPtr);
        builder->CreateRetVoid();
    }

    llvm::verifyFunction(*function, &llvm::errs());

    scope->declareVar(fn->name, function);
    return function;
}

llvm::Value* Compiler::genVarDecl(std::shared_ptr<VarDeclarationType> decl, std::shared_ptr<Scope> scope)
{
    std::string varName = decl->identifier;

    llvm::AllocaInst* alloca = builder->CreateAlloca(valuetype, nullptr, varName);

    if (decl->value && decl->value->kind != NodeType::UndefinedLiteral)
    {
        llvm::Value* initialValue = gen(decl->value, scope);
        builder->CreateStore(initialValue, alloca);
    }
    else
    {
        builder->CreateStore(llvm::Constant::getNullValue(valuetype), alloca);
    }

    scope->declareVar(varName, alloca);

    return alloca;
}

llvm::Value* Compiler::genIdent(std::shared_ptr<IdentifierType> ident, std::shared_ptr<Scope> scope)
{
    llvm::Value* val = scope->lookUp(ident->symbol);

    if (llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(val))
    {
        val = builder->CreateLoad(valuetype, alloca, "loaded_return_val");
    }

    return val;
}

llvm::Value* Compiler::genNum(std::shared_ptr<NumericLiteralType> num, std::shared_ptr<Scope> scope)
{
    double number = num->numValue;
    union DoubleInt {
        double d;
        uint64_t i64;
    } u;

    u.d = number;

    uint32_t low = (uint32_t)(u.i64 & 0xFFFFFFFF);
    uint32_t high = (uint32_t)(u.i64 >> 32);

    std::vector<llvm::Constant*> elems(16, llvm::ConstantInt::get(builder->getInt32Ty(), 0));
    elems[0] = llvm::ConstantInt::get(builder->getInt32Ty(), low);
    elems[1] = llvm::ConstantInt::get(builder->getInt32Ty(), high);

    auto payloadConst = llvm::ConstantArray::get(llvm::ArrayType::get(builder->getInt32Ty(), 16), elems);

    auto tagConst = llvm::ConstantInt::get(builder->getInt32Ty(), static_cast<int>(ValueTag::Number));

    auto result = llvm::ConstantStruct::get(
        llvm::cast<llvm::StructType>(valuetype),
        { tagConst, payloadConst }
    );

    return result;
}

llvm::Value* Compiler::genCall(std::shared_ptr<CallExprType> call, std::shared_ptr<Scope> scope)
{
    llvm::Value* calee = gen(call->calee, scope);
    llvm::Function* fn = llvm::dyn_cast<llvm::Function>(calee);
    if (!fn)
    {
        throw std::runtime_error(CustomError("Only functions and probes can be called", "CallError", call->calee->token));
    }
    llvm::AllocaInst* result = builder->CreateAlloca(valuetype, nullptr, "retval");
    std::vector<llvm::Value*> args = { result };
    for (auto& arg : call->args)
    {
        args.push_back(gen(arg, scope));
    }
    builder->CreateCall(fn, llvm::ArrayRef(args));
   
    return builder->CreateLoad(valuetype, result, "loaded_retval");
}