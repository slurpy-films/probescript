#pragma once
#include "runtime/values.hpp"
#include "ast.hpp"
#include "config.hpp"
#include "stdlib/stdlib.hpp"

Val evalImportStmt(ImportStmtType* importstmt, Env* envptr, Config::Config* config) {
    std::string modulename = importstmt->name;
    std::unordered_map<std::string, std::shared_ptr<ObjectVal>> stdlib = getStdlib();
    if (stdlib.find(modulename) != stdlib.end()) {
        if (importstmt->hasMember) {
            Expr* member = importstmt->module;
            Env* modEnv = new Env();
            modEnv->declareVar(modulename, stdlib[modulename]);
            envptr->declareVar(importstmt->customIdent ? importstmt->ident : static_cast<MemberExprType*>(importstmt->module)->lastProp, eval(member, modEnv));
        } else envptr->declareVar(importstmt->customIdent ? importstmt->ident : modulename, stdlib[modulename]);
        return std::make_shared<UndefinedVal>();
    }

    if (config->modules.find(modulename) == config->modules.end()) {
        std::cerr << "Cannot find module " << modulename;
        exit(1);
    }

    fs::path filepath = config->modules[modulename];

    std::ifstream stream(filepath);

    std::string file((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());

    Parser parser;

    ProgramType* program = parser.produceAST(file);

    Config::Config* conf = new Config::Config(Config::Exports);

    conf->modules = config->modules;

    Val evaluated = eval(program, new Env(), conf);

    std::shared_ptr<ObjectVal> moduleObj = std::make_shared<ObjectVal>(evaluated->exports);

    if (importstmt->hasMember) {
        Expr* member = importstmt->module;
        Env* modEnv = new Env();
        modEnv->declareVar(modulename, moduleObj);
        envptr->declareVar(importstmt->customIdent ? importstmt->ident : static_cast<MemberExprType*>(importstmt->module)->lastProp, eval(member, modEnv));
    } else envptr->declareVar(importstmt->customIdent ? importstmt->ident : modulename, moduleObj, true);

    return std::make_shared<UndefinedVal>();
}
