#include "compiler.hpp"

using namespace probescript;

Compiler::Compiler(CompilationTarget target)
    : target(target)
{
    setup();
};

void Compiler::setup()
{
    // Initialize member variables
    ctx = std::make_unique<llvm::LLVMContext>();
    module = std::make_unique<llvm::Module>("Probescript", *ctx);
    builder = std::make_unique<llvm::IRBuilder<>>(*ctx);

    // x86 and x86_64
    LLVMInitializeX86Target();
    LLVMInitializeX86TargetMC();
    LLVMInitializeX86TargetInfo();
    LLVMInitializeX86AsmParser();
    LLVMInitializeX86AsmPrinter();

    // ARM
    LLVMInitializeARMTarget();
    LLVMInitializeARMTargetMC();
    LLVMInitializeARMTargetInfo();
    LLVMInitializeARMAsmParser();
    LLVMInitializeARMAsmPrinter();

    // AArch64 (ARM64)
    LLVMInitializeAArch64Target();
    LLVMInitializeAArch64TargetMC();
    LLVMInitializeAArch64TargetInfo();
    LLVMInitializeAArch64AsmParser();
    LLVMInitializeAArch64AsmPrinter();

    std::string triple;
    std::string cpu = "generic";
    std::string features = "";
    
    switch (target)
    {
    case CompilationTarget::WindowsX64:
        triple = "x86_64-pc-windows-msvc";
        break;
    case CompilationTarget::WindowsARM64:
        triple = "aarch64-pc-windows-msvc";
        break;
    case CompilationTarget::LinuxX64:
        triple = "x86_64-pc-linux-gnu";
        break;
    default:
        triple = llvm::sys::getDefaultTargetTriple();
        break;
    }

    module->setTargetTriple(triple);

    std::string error;
    const llvm::Target* targetDesc = llvm::TargetRegistry::lookupTarget(triple, error);
    if (!targetDesc)
    {
        throw std::runtime_error("Could not lookup LLVM target: " + error);
    }

    llvm::TargetOptions opt;
    auto relocModel = llvm::Reloc::PIC_;

    targetMachine = std::unique_ptr<llvm::TargetMachine>(targetDesc->createTargetMachine(triple, cpu, features, opt, relocModel));

    if (!targetMachine) {
        throw std::runtime_error("Could not create target machine for triple: " + triple);
    }

    module->setDataLayout(targetMachine->createDataLayout());

    // 'valuetype' represents a user-defined value
    valuetype = llvm::StructType::create(*ctx, "_value");
    
    std::vector<llvm::Type*> fields = {
        builder->getInt32Ty(), // tag: ValueTag
        llvm::ArrayType::get(builder->getInt32Ty(), 16) // Payload: Value payload
    };

    valuetype->setBody(fields, false);

    // Declare functions from probescript-runtime

    // System exit, takes a single i32 as a parameter and returns void
    auto exitFnty = llvm::FunctionType::get(
        llvm::Type::getVoidTy(*ctx),
        { llvm::Type::getInt32Ty(*ctx) },
        false
    );
    
    auto exitFn = llvm::Function::Create(
        exitFnty,
        llvm::Function::ExternalLinkage,
        "exit",
        module.get()
    );

    // Wraps the system exit function
    llvm::FunctionType* exitWrapperTy = llvm::FunctionType::get(
        builder->getVoidTy(),
        { llvm::PointerType::getUnqual(valuetype), valuetype },
        false
    );
    llvm::Function* exitWrapper = llvm::Function::Create(
        exitWrapperTy,
        llvm::Function::ExternalLinkage,
        "_exit",
        module.get()
    );
    llvm::BasicBlock* exitEntry = createBB("entry", exitWrapper);
    builder->SetInsertPoint(exitEntry);

    llvm::Function::arg_iterator args = exitWrapper->arg_begin();
    llvm::Value* retPtr = args++;
    retPtr->setName("__sret");
    llvm::Value* codeValue = args++;
    codeValue->setName("code");

    llvm::Value* payloadArray = builder->CreateExtractValue(codeValue, {1}, "payload_array");

    llvm::Value* exitCode = builder->CreateExtractValue(payloadArray, {0}, "exit_code");

    builder->CreateCall(exitFn, { exitCode });
    builder->CreateRetVoid();
}

void Compiler::compile(std::shared_ptr<ProgramType> program)
{
    genProgram(program);

    auto startFnType = llvm::FunctionType::get(builder->getVoidTy(), false);
    auto startFn = llvm::Function::Create(
        startFnType,
        llvm::Function::ExternalLinkage,
        "_start",
        *module
    );

    auto entry = llvm::BasicBlock::Create(*ctx, "entry", startFn);
    builder->SetInsertPoint(entry);

    auto probescriptMain = module->getFunction("Main");
    if (!probescriptMain)
    {
        throw std::runtime_error(CustomError("'Main' function not found in the module", "CompilerError"));
    }

    llvm::AllocaInst* retVal = builder->CreateAlloca(valuetype, nullptr, "retval");
    retVal->setAlignment(llvm::Align(8));

    builder->CreateCall(probescriptMain, { retVal });

    llvm::Value* payloadPtr = builder->CreateStructGEP(valuetype, retVal, 1, "payloadPtr");

    llvm::PointerType* doublePtrType = builder->getDoubleTy()->getPointerTo();
    llvm::Value* doublePtr = builder->CreateBitCast(payloadPtr, doublePtrType, "doublePtr");

    llvm::Value* loadedPayload = builder->CreateLoad(builder->getDoubleTy(), doublePtr, "loadedPayload");

    llvm::Value* exitCode = builder->CreateFPToSI(loadedPayload, builder->getInt32Ty(), "exitCode");

    auto exitfn = module->getFunction("exit");
    if (!exitfn)
    {
        throw std::runtime_error(CustomError("'exit' function not found in the module", "CompilerError"));
    }

    builder->CreateCall(exitfn, { exitCode });
    builder->CreateRetVoid();
}

void Compiler::dump(std::filesystem::path path)
{
    module->print(llvm::outs(), nullptr); // debug
    
    std::string error;
    llvm::raw_string_ostream errorStream(error);
    if (llvm::verifyModule(*module, &errorStream))
    {
        throw std::runtime_error("Compiler error: " + error);
    }   
    llvm::legacy::PassManager pm;
    auto filetype = llvm::CodeGenFileType::ObjectFile;
    std::error_code EC;

    llvm::raw_fd_ostream dest(path.string(), EC);
    if (EC)
        throw std::runtime_error("Could not open file to write object");

    if (targetMachine->addPassesToEmitFile(pm, dest, nullptr, filetype)) {
        throw std::runtime_error("TargetMachine can't emit object file");
    }
    
    pm.run(*module);
    dest.flush();
}

llvm::Function* Compiler::createFunction(const std::string& name, llvm::FunctionType* fntype)
{
    auto fn = module->getFunction(name);

    if (!fn)
    {
        fn = createFunctionProto(name, fntype);
    }

    createFunctionBlock(fn);

    return fn;
}

llvm::Function* Compiler::createFunctionProto(const std::string& name, llvm::FunctionType* fntype)
{
    auto fn = llvm::Function::Create(fntype, llvm::Function::ExternalLinkage, name, *module);

    llvm::verifyFunction(*fn, &llvm::errs());

    return fn;
}

void Compiler::createFunctionBlock(llvm::Function* fn)
{
    auto entry = createBB("entry", fn);
    builder->SetInsertPoint(entry);
}

llvm::BasicBlock* Compiler::createBB(std::string name, llvm::Function* fn)
{
    return llvm::BasicBlock::Create(*ctx, name, fn);
}

Compiler::Scope::Scope(llvm::Module* module, llvm::IRBuilder<>* builder, std::shared_ptr<Scope> parent)
{
    llvm::Function* exitFn = module->getFunction("_exit");
    m_parent = parent;

    declareVar("exit", exitFn);
}

std::shared_ptr<Compiler::Scope> Compiler::Scope::getParent()
{
    return m_parent;
}

llvm::Value* Compiler::Scope::declareVar(const std::string& name, llvm::Value* val)
{
    m_variables[name] = val;
    return val;
}

llvm::Value* Compiler::Scope::lookUp(const std::string& name)
{
    if (m_variables.find(name) != m_variables.end())
    {
        return m_variables[name];
    }
    else if (m_parent)
    {
        return m_parent->lookUp(name);
    }
    else
    {
        throw std::runtime_error(CustomError("Variable " + name + " is not defined", "ReferenceError"));
    }
}