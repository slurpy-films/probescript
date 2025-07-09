#include "compiler.hpp"

using namespace probescript;

void Compiler::genProgram(std::shared_ptr<ProgramType> program)
{
    m_currentscope = std::make_shared<Scope>();
    for (auto stmt : program->body)
    {
        gen(stmt);
    }
}

llvm::Value* Compiler::gen(std::shared_ptr<Stmt> node)
{
    if (!node)
        throw std::runtime_error("gen() called with nullptr");
    
        switch (node->kind)
    {
        case NodeType::NumericLiteral:
            return genNum(std::static_pointer_cast<NumericLiteralType>(node));
        case NodeType::ReturnStmt:
            return genReturn(std::static_pointer_cast<ReturnStmtType>(node));
        case NodeType::VarDeclaration:
            return genVarDecl(std::static_pointer_cast<VarDeclarationType>(node));
        case NodeType::FunctionDeclaration:
            return genFunction(std::static_pointer_cast<FunctionDeclarationType>(node));
        case NodeType::Identifier:
            return genIdent(std::static_pointer_cast<IdentifierType>(node));
        default:
            throw std::runtime_error("Unknown node type in gen()");
    }
}

llvm::Value* Compiler::genReturn(std::shared_ptr<ReturnStmtType> stmt)
{
    if (!m_retptr)
    {
        throw std::runtime_error(CustomError("Did not expect return statement", "ReturnError", stmt->token));
    }

    llvm::Value* val = gen(stmt->val);

    if (llvm::AllocaInst* alloca = llvm::dyn_cast<llvm::AllocaInst>(val)) {
        val = builder->CreateLoad(valuetype, alloca, "loaded_return_val");
    }

    llvm::Value* retptr = builder->CreateBitCast(m_retptr, llvm::PointerType::getUnqual(valuetype));

    llvm::Value* tagGEP = builder->CreateStructGEP(valuetype, retptr, 0, "ret_tag");
    llvm::Value* tagVal = builder->CreateExtractValue(val, {0});
    builder->CreateStore(tagVal, tagGEP);

    llvm::Value* payloadGEP = builder->CreateStructGEP(valuetype, retptr, 1, "ret_payload");
    llvm::Value* payloadVal = builder->CreateExtractValue(val, {1});
    builder->CreateStore(payloadVal, payloadGEP);

    return builder->CreateRetVoid();
}

llvm::Value* Compiler::genProbe(std::shared_ptr<ProbeDeclarationType> prb)
{
    auto fn = createFunction(prb->name, llvm::FunctionType::get(valuetype, false));
    
    for (auto stmt : prb->body)
    {
        gen(stmt);
    }

    return fn;
}

llvm::Value* Compiler::genFunction(std::shared_ptr<FunctionDeclarationType> fn)
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

    for (; argIt != function->arg_end(); ++argIt)
    {
        argIt->setName(fn->parameters[idx]->identifier);
        llvm::AllocaInst* alloc = builder->CreateAlloca(valuetype, nullptr, argIt->getName());

        builder->CreateStore(&(*argIt), alloc);

        m_currentscope->declareVar(fn->parameters[idx]->identifier, alloc);

        idx++;
    }

    m_currentscope = std::make_shared<Scope>(m_currentscope);
    for (auto stmt : fn->body)
    {
        gen(stmt);
    }
    
    m_currentscope = m_currentscope->getParent();

    if (!builder->GetInsertBlock()->getTerminator())
    {
        llvm::Value* tagGEP = builder->CreateStructGEP(valuetype, m_retptr, 0, "sret_tag");
        builder->CreateStore(llvm::ConstantInt::get(builder->getInt32Ty(), 0), tagGEP);

        llvm::Value* payloadGEP = builder->CreateStructGEP(valuetype, m_retptr, 1, "sret_payload");
        llvm::Value* zeroDouble = llvm::ConstantFP::get(builder->getDoubleTy(), 0.0);

        llvm::Value* castedPtr = builder->CreateBitCast(payloadGEP, builder->getDoubleTy()->getPointerTo());
        builder->CreateStore(zeroDouble, castedPtr);
    }

    llvm::verifyFunction(*function, &llvm::errs());

    return function;
}

llvm::Value* Compiler::genVarDecl(std::shared_ptr<VarDeclarationType> decl)
{
    std::string varName = decl->identifier;

    llvm::AllocaInst* alloca = builder->CreateAlloca(valuetype, nullptr, varName);

    if (decl->value && decl->value->kind != NodeType::UndefinedLiteral)
    {
        llvm::Value* initialValue = gen(decl->value);
        builder->CreateStore(initialValue, alloca);
    }
    else
    {
        builder->CreateStore(llvm::Constant::getNullValue(valuetype), alloca);
    }

    m_currentscope->declareVar(varName, alloca);

    return alloca;
}

llvm::Value* Compiler::genIdent(std::shared_ptr<IdentifierType> ident)
{
    return m_currentscope->lookUp(ident->symbol);
}

llvm::Value* Compiler::genNum(std::shared_ptr<NumericLiteralType> num)
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

    auto tagConst = llvm::ConstantInt::get(builder->getInt32Ty(), 0);

    auto result = llvm::ConstantStruct::get(
        llvm::cast<llvm::StructType>(valuetype),
        { tagConst, payloadConst }
    );

    return result;
}