#pragma once
#include <string>
#include <memory>
#include <filesystem>
#include <unordered_map>

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/IR/InlineAsm.h>

#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/MC/TargetRegistry.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/Error.h>

#include "core/frontend/ast.hpp"
#include "core/errors.hpp"

namespace probescript
{
    
enum class CompilationTarget
{
    WindowsX64,
    WindowsARM64,
    LinuxX64,

    Native
};

class Compiler
{
public:
    Compiler(CompilationTarget target);
    void compile(std::shared_ptr<ProgramType> program);

    void dump(std::filesystem::path path);
private:
    CompilationTarget target;

    std::unique_ptr<llvm::LLVMContext> ctx;
    std::unique_ptr<llvm::Module> module;
    std::unique_ptr<llvm::IRBuilder<>> builder;
    std::unique_ptr<llvm::TargetMachine> targetMachine;

    llvm::StructType* valuetype;

    llvm::Function* createFunction(const std::string& name, llvm::FunctionType* fntype);
    llvm::Function* createFunctionProto(const std::string& name, llvm::FunctionType* fntype);

    void createFunctionBlock(llvm::Function* fn);
    llvm::BasicBlock* createBB(std::string name, llvm::Function* fn = nullptr);
    void setup();

    void genProgram(std::shared_ptr<ProgramType> program);
    
    llvm::Value* gen(std::shared_ptr<Stmt> node);
    
    // Statement generator functions
    llvm::Value* genReturn(std::shared_ptr<ReturnStmtType> stmt);
    llvm::Value* genProbe(std::shared_ptr<ProbeDeclarationType> prb);
    llvm::Value* genFunction(std::shared_ptr<FunctionDeclarationType> fn);
    llvm::Value* genVarDecl(std::shared_ptr<VarDeclarationType> decl);
    
    // Expression generator functions
    llvm::Value* genIdent(std::shared_ptr<IdentifierType> ident);
    llvm::Value* genNum(std::shared_ptr<NumericLiteralType> num);

    llvm::Value* m_retptr; // Used for sret in user-defined functions

    class Scope : std::enable_shared_from_this<Scope>
    {
    public:
        Scope(std::shared_ptr<Scope> parent = nullptr);

        llvm::Value* declareVar(const std::string& name, llvm::Value* val); // llvm::Value* return type since it returns the 'val' argument
        llvm::Value* lookUp(const std::string& name);

        std::shared_ptr<Scope> getParent();
    private:
        std::unordered_map<std::string, llvm::Value*> m_variables;
        std::shared_ptr<Scope> m_parent;
    };

    std::shared_ptr<Scope> m_currentscope;
};
}