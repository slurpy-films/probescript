#include "globals.hpp"

using namespace Probescript;

std::unordered_map<std::string, Typechecker::TypePtr> g_typeGlobals =
{
    {
        "console",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Module, "object", std::make_shared<Typechecker::TypeVal>(std::unordered_map<std::string, Typechecker::TypePtr>({ { "println", std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function") }, { "print", std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function") }, { "prompt", std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function")} })))
    },
    {
        "num",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Number, "number")))
    },
    {
        "str",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::make_shared<Typechecker::Type>(Typechecker::TypeKind::String, "string")))
    },
    {
        "bool",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Bool, "boolean")))
    },
    {
        "map",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Object, "map")))
    },
    {
        "function",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function")))
    },
    {
        "future",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Future, "future")))
    },
    {
        "array",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Array, "array")))
    },
    {
        "exit",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Any, "native module", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("code", Typechecker::g_numty, true) })))
    },
    {
        "keys",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("obj", Typechecker::g_anyty, false) })))
    },
    {
        "values",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("obj", Typechecker::g_anyty, false) })))
    },
    {
        "copy",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("val", Typechecker::g_anyty, false) })))
    },
    {
        "evaluate",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("input", Typechecker::g_strty, false) })))
    },
    {
        "sleep",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("milliseconds", Typechecker::g_numty, false) })))
    },
    {
        "Regex",
        std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Class, "native class", std::make_shared<Typechecker::TypeVal>(std::unordered_map<std::string, Typechecker::TypePtr>(
            {
                {
                    "test",
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("input", Typechecker::g_strty, false) })))
                },
                {
                    "search",
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("input", Typechecker::g_strty, false) })))
                },
                {
                    "replace",
                    std::make_shared<Typechecker::Type>(Typechecker::TypeKind::Function, "function", std::make_shared<Typechecker::TypeVal>(std::vector({ std::make_shared<Typechecker::Parameter>("input", Typechecker::g_strty, false), std::make_shared<Typechecker::Parameter>("replacement", Typechecker::g_strty, false) })))
                }
            }
        )), "Regex")
    }
};

std::unordered_map<std::string, VM::ValuePtr> g_valueGlobals =
{
    {
        "num",
        std::make_shared<VM::NativeClassVal>([](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
        {
            if (!args.empty() && !isNum(args[0]->toString()))
            {
                throw ThrowException(ArgumentError("Invalid argument: '" + args[0]->toString() + "' is not a number"));
            }

            return std::make_shared<VM::NumberVal>(!args.empty() ? args[0]->toNum() : 0);
        })
    },
    {
        "str",
        std::make_shared<VM::NativeClassVal>([](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
        {
            return std::make_shared<VM::StringVal>(!args.empty() ? args[0]->toString() : "");
        })
    },
    {
        "bool",
        std::make_shared<VM::NativeClassVal>([](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
        {
            return std::make_shared<VM::BooleanVal>(!args.empty() ? args[0]->toBool() : false);
        })
    },
    {
        "map",
        std::make_shared<VM::NativeClassVal>([](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
        {
            return std::make_shared<VM::ObjectVal>(std::unordered_map<std::string, VM::ValuePtr>());
        })
    },
    {
        "function",
        std::make_shared<VM::NativeClassVal>([](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
        {
            if (args.empty() || args[0]->type == VM::ValueType::Function) throw ThrowException(ArgumentError("Usage: new function(fn: function)"));

            return std::static_pointer_cast<VM::FunctionValue>(args[0]);
        })
    },
    {
        "future",
        std::make_shared<VM::NativeClassVal>([](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
        {
            return std::make_shared<VM::NullVal>();
        })
    },
    {
        "array",
        std::make_shared<VM::NativeClassVal>([](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
        {
            std::shared_ptr<VM::ArrayVal> array = std::make_shared<VM::ArrayVal>();

            for (const auto val : args)
            {
                array->items.push_back(val);
            }

            return array;
        })
    },
    {
        "exit",
        std::make_shared<VM::NativeFunctionVal>([](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
        {
            exit(args.empty() ? 0 : args[0]->toNum());
            return std::make_shared<VM::NullVal>();
        })
    },
    {
        "keys",
        std::make_shared<VM::NativeFunctionVal>([](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
        {
            if (args.empty() || args[0]->type != VM::ValueType::Object) throw ThrowException(ArgumentError("Usage: keys(obj: Object)"));

            std::shared_ptr<VM::ArrayVal> array = std::make_shared<VM::ArrayVal>();

            for (const auto& [key, _] : std::static_pointer_cast<VM::ObjectVal>(args[0])->properties)
            {
                array->items.push_back(std::make_shared<VM::StringVal>(key));
            }

            return array;
        })
    },
    {
        "values",
        std::make_shared<VM::NativeFunctionVal>([](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
        {
            if (args.empty() || args[0]->type != VM::ValueType::Object) throw ThrowException(ArgumentError("Usage: values(obj: Object)"));

            std::shared_ptr<VM::ArrayVal> array = std::make_shared<VM::ArrayVal>();

            for (const auto& [_, val] : std::static_pointer_cast<VM::ObjectVal>(args[0])->properties)
            {
                array->items.push_back(val);
            }

            return array;
        })
    },
    {
        "copy",
        std::make_shared<VM::NativeFunctionVal>([](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
        {
            if (args.empty()) throw ThrowException(ArgumentError("Usage: copy(val: any)"));

            return std::make_shared<VM::Value>(*args[0]);
        })
    },
    {
        "evaluate",
        std::make_shared<VM::NativeFunctionVal>([](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
        {
            if (args.empty()) throw ThrowException(ArgumentError("Usage: evaluate(val: string)"));

            try
            {
                std::string code = args[0]->toString();
                auto parsed = Parser().parse(code);
                
                Compiler compiler(parsed);
                compiler.compile();

                VM::Machine vm(compiler.getInstructions(), compiler.getConstants(), std::make_shared<VM::Scope>());
                std::make_shared<Values::UndefinedVal>();
            }
            catch (const std::runtime_error& err)
            {
                throw ThrowException(err.what());
            }

            return std::make_shared<VM::NullVal>();
        })
    },
    {
        "sleep",
        std::make_shared<VM::NativeFunctionVal>([](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
        {
            // This function is disabled for now
            // When futures are added to the VM, this function will be re-added

            return std::make_shared<VM::NullVal>();
        })
    },
    {
        "Regex",
        std::make_shared<VM::NativeClassVal>([](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
        {
            if (args.empty()) throw ThrowException(ArgumentError("Usage: new Regex(expression: string)"));

            std::regex regex(args[0]->toString());
            std::shared_ptr<VM::ObjectVal> obj = std::make_shared<VM::ObjectVal>();

            obj->properties["test"] = std::make_shared<VM::NativeFunctionVal>([regex](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
            {
                if (args.empty()) throw ThrowException(ArgumentError("Usage: regex.match(input: string)"));

                return std::make_shared<VM::BooleanVal>(std::regex_match(args[0]->toString(), regex));
            });

            obj->properties["search"] = std::make_shared<VM::NativeFunctionVal>([regex](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
            {
                if (args.empty()) throw ThrowException(ArgumentError("Usage: regex.search(input: string)"));
                return std::make_shared<VM::BooleanVal>(std::regex_search(args[0]->toString(), regex));
            });

            obj->properties["replace"] = std::make_shared<VM::NativeFunctionVal>([regex](std::vector<VM::ValuePtr> args) -> VM::ValuePtr
            {
                if (args.size() < 2) throw ThrowException(ArgumentError("Usage: regex.replace(input: string, replacement: string)"));
                std::string result = std::regex_replace(args[0]->toString(), regex, args[1]->toString());
                return std::make_shared<VM::StringVal>(result);
            });

            return obj;
        })
    }
};

// This will be removed but the legacy runtime depends on it
std::unordered_map<std::string, std::pair<Probescript::Values::Val, Probescript::Typechecker::TypePtr>> g_globals = {};