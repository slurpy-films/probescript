#pragma once
#include <string>
#include "runtime/values.hpp"
#include "frontend/lexer.hpp"
#include "context.hpp"
#include "utils.hpp"

namespace Probescript
{

std::string Error(const std::string& m);
std::string SyntaxError(const std::string& m, Lexer::Token tk = Lexer::Token(), std::shared_ptr<Context> ctx = std::make_shared<Context>());
std::string TypeError(const std::string& m, Lexer::Token tk = Lexer::Token());
std::string ArgumentError(const std::string& m);
std::string CustomError(const std::string& m, const std::string& n);
std::string CustomError(const std::string& m, const std::string& n, Lexer::Token tk);

} // namespace Probescript