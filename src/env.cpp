#include "runtime/env.hpp"

Env::Env(Env* parentENV)  {
    bool global = (parentENV == nullptr);
    parent = parentENV;

    if (global) {
        declareVar("true", std::make_shared<BooleanVal>(true), true);
        declareVar("false", std::make_shared<BooleanVal>(false), true);
        declareVar("num", std::make_shared<NativeFnValue>(toNum), true);
        declareVar("str", std::make_shared<NativeFnValue>(toStr), true);
        declareVar("console", std::make_shared<ObjectVal>(getConsole()), true);
        declareVar("process", std::make_shared<ObjectVal>(getProcessModule()));
        declareVar("undefined", std::make_shared<UndefinedVal>(), true);
    }
}

Val Env::declareVar(std::string varName, Val value, bool constant) {
    if (variables.find(varName) != variables.end()) {
        std::cout << "Variable " << varName << " is already defined" << std::endl;
        exit(1);
    }

    variables[varName] = value;
    constants[varName] = constant;
    return value;
}

Val Env::assignVar(std::string varName, Val value) {
    Env* env = resolve(varName);

    if (env->constants.find(varName) != env->constants.end() && env->constants[varName]) {
        std::cout << "Assignment to constant variable " << varName << std::endl;
        exit(1);
    }

    env->variables[varName] = value;

    return value;
}

Val Env::lookupVar(std::string varName) {
    Env* env = resolve(varName);
    return env->variables[varName];
}


std::shared_ptr<UndefinedVal> Env::throwErr(std::string err) {
    if (hasCatch) {
        evalCallWithFnVal(catcher, { std::make_shared<StringVal>(err) }, this);
    } else if (parent) {
        return parent->throwErr(err);
    } else {
        std::cerr << err;
        exit(1);
    }

    return std::make_shared<UndefinedVal>();
}

Env* Env::resolve(std::string varname) {
    if (variables.find(varname) != variables.end()) {
        return this;
    }

    if (!parent) {
        std::cout << "Cannot resolve variable " << varname << " as it does not exist";
        exit(1);
    }

    return parent->resolve(varname);
}

void Env::setCatch(Val fn) {
    hasCatch = true;
    catcher = fn;
}