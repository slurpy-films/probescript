// This file contains the definitions of the function and structs declared in 'probescript.h'

#include "probescript.h"

#include <string>
#include <memory>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>

#include "core/context.hpp"
#include "core/vm/vm.hpp"
#include "core/vm/values.hpp"
#include "core/compiler/compiler.hpp"
#include "core/frontend/parser.hpp"
#include "core/frontend/ast.hpp"

using namespace Probescript;

Prb_Value *valueptr_to_prbvalue(VM::ValuePtr value)
{
    auto val = new Prb_Value();
    switch (value->type)
    {
        case VM::ValueType::String:
            val->type = Prb_String;
            val->as.string = strdup(value->toString().c_str());
            break;
        case VM::ValueType::Number:
            val->type = Prb_Number;
            val->as.number = value->toNum();
            break;

        default:
            val->type = Prb_Null;
            break;
    }

    return val;
}

static VM::ValuePtr wrap_cfunc(Prb_CFunction cfunc);

VM::ValuePtr prbvalue_to_valueptr(Prb_Value *value)
{
    VM::ValuePtr result;

    switch (value->type)
    {
        case Prb_String:
            result = std::make_shared<VM::StringVal>(std::string(value->as.string));
            break;
        case Prb_Number:
            result = std::make_shared<VM::NumberVal>(value->as.number);
            break;
        case Prb_Function:
            result = wrap_cfunc(value->as.function);
            break;

        default:
            result = std::make_shared<VM::NullVal>();
    }

    return result;
}

VM::ValuePtr wrap_cfunc(Prb_CFunction cfunc)
{
    return std::make_shared<VM::NativeFunctionVal>(
        [cfunc](std::vector<VM::ValuePtr> args, std::shared_ptr<VM::FunctionContext>) -> VM::ValuePtr
        {
            std::vector<Prb_Value*> cargs;
            cargs.reserve(args.size());
            for (auto &arg : args)
            {
                cargs.push_back(valueptr_to_prbvalue(arg));
            }

            Prb_Value *ret = cfunc((int)cargs.size(), cargs.data());

            return prbvalue_to_valueptr(ret);
        }
    );
}

struct Prb_VM
{
    VM::Machine *machine;
};

struct Prb_Compiler
{
    Parser *parser;

    std::shared_ptr<Context> context;
};

Prb_Compiler *prb_create_compiler(const char *filename)
{
    Parser *parser = new Parser();
    
    std::string strFilename = std::string(filename);

    std::ifstream stream(filename);
    std::string file((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    
    auto context = std::make_shared<Context>();
    context->file = file;
    context->filename = strFilename;
    context->modules = {};

    Prb_Compiler *compiler = new Prb_Compiler();
    compiler->context = context;
    compiler->parser = parser;

    return compiler;
}

Prb_VM *prb_create_vm()
{
    Prb_VM *vm = new Prb_VM();
    vm->machine = new VM::Machine();
    return vm;
}

void prb_run(Prb_VM *vm, Prb_Compiler *compiler)
{
    auto parsed = compiler->parser->parse(compiler->context->file, compiler->context);
    
    auto bcCompiler = Compiler(parsed, compiler->context);

    for (const auto &[key, val] : vm->machine->getGlobals())
    {
       bcCompiler.registerGlobal(key);
    }

    bcCompiler.compile();

    auto bytecode = bcCompiler.getInstructions();
    auto constants = bcCompiler.getConstants();

    vm->machine->load(bytecode, constants);
    vm->machine->run();
}

void prb_register_fn(Prb_VM *vm, const char *name, Prb_CFunction fn)
{
    if (!vm || !name || !fn) return;
    std::string strName(name);
    auto value = wrap_cfunc(fn);
    vm->machine->registerGlobal(strName, value);
}

Prb_Value *prb_call_fn(Prb_VM *vm, const char *name, int argc, Prb_Value **argv)
{
    VM::ValuePtr val;
    try
    {
        val = vm->machine->lookup(std::string(name));
    }
    catch(...)
    {
        // The most likely error we get here is that the function name does not
        // exist in the current scope, so we just assume that
        Prb_Value *error = new Prb_Value();
        error->type = Prb_Error;
        error->as.error = Prb_Error_NotExists;
        
        return error;
    }

    if (val->type != VM::ValueType::Function)
    {
        Prb_Value *error = new Prb_Value();
        error->type = Prb_Error;
        error->as.error = Prb_Error_NotAFunction;

        return error;
    }

    std::vector<VM::ValuePtr> args;
    args.reserve(argc);

    for (int i = 0; i < argc; ++i)
    {
        args.push_back(prbvalue_to_valueptr(argv[i]));
    }

    auto ret = VM::call(val, args, std::make_shared<VM::FunctionContext>(vm->machine->getConstants(), vm->machine->getGlobals()));
    return valueptr_to_prbvalue(ret);
}

void prb_destroy_vm(Prb_VM *vm)
{
    delete vm->machine;
    delete vm;
}

void prb_destroy_compiler(Prb_Compiler *compiler)
{
    delete compiler->parser;
    delete compiler;
}