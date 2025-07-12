#include "errors.hpp"

std::string Error(const std::string& m)
{
    return ConsoleColors::RED + "[Error]: " + ConsoleColors::RESET + m + "\n";
}

std::string SyntaxError(const std::string& m, Lexer::Token tk, std::shared_ptr<Context> ctx)
{
    std::vector<std::string> file = split(ctx->file, "\n");
    std::string line = file[tk.line - 1];
    
    if (line == "")
    {
        return ConsoleColors::RED + "[SyntaxError]: " + ConsoleColors::RESET + m + "\n";
    }
    else
    {
        // Make the error message
        // pointer: points to the token where the error occured

        std::string pointer;
        std::string nextline = (tk.line < file.size()) ? file[tk.line] : "";
        std::string lastline = (tk.line > 1) ? file[tk.line - 2] + "\n" : "";

        size_t len = line.size();

        for (size_t i = 0; i < tk.col - 1 && i < len; i++)
        {
            pointer += (line[i] == '\t' ? "\t" : " ");
        }

        pointer += ConsoleColors::RED;

        for (size_t i = 0; i < tk.value.size(); i++)
            pointer += "^";

        pointer += ConsoleColors::RESET + "\n";

        return
            ConsoleColors::RED + "[SyntaxError]: " + ConsoleColors::RESET + m + "\n\n"
            + "At " + ctx->filename + ":" + std::to_string(tk.line) + ":" + std::to_string(tk.col) + "\n"
            + lastline
            + line + "\n"
            + pointer
            + nextline + "\n";
    }
}

std::string TypeError(const std::string& m, Lexer::Token tk)
{
    std::vector<std::string> file = split(tk.ctx->file, "\n");

    std::string line = file[tk.line - 1];
    if (line.empty())
    {
        return ConsoleColors::RED + "[TypeError]: " + ConsoleColors::RESET + m + "\n";
    }
    else
    {
        // Make the error message
        // pointer: points to the token where the error occured

        std::string pointer;
        std::string nextline = (tk.line < file.size()) ? file[tk.line] : "";
        std::string lastline = (tk.line > 1) ? file[tk.line - 2] + "\n" : "";

        size_t len = line.size();

        for (size_t i = 0; i < tk.col - 1 && i < len; i++)
        {
            pointer += (line[i] == '\t' ? "\t" : " ");
        }

        pointer += ConsoleColors::RED;

        for (size_t i = 0; i < tk.value.size(); i++)
            pointer += "^";

        pointer += ConsoleColors::RESET + "\n";

        return
            ConsoleColors::RED + "[TypeError]: " + ConsoleColors::RESET + m + "\n\n"
            + "At " + tk.ctx->filename + ":" + std::to_string(tk.line) + ":" + std::to_string(tk.col) + "\n"
            + lastline
            + line + "\n"
            + pointer
            + nextline + "\n";
    }
}

std::string ArgumentError(const std::string& m)
{
    return ConsoleColors::RED + "[ArgumentError]: " + ConsoleColors::RESET + m + "\n";
}

std::string CustomError(const std::string& m, const std::string& n)
{
    return ConsoleColors::RED + "[" + n + "]: " + ConsoleColors::RESET + m + "\n";
}

std::string CustomError(const std::string& m, const std::string& n, Lexer::Token tk)
{
    std::vector<std::string> file = split(tk.ctx->file, "\n");

    std::string line = file[tk.line - 1];
    if (line.empty())
    {
        return ConsoleColors::RED + "[" + n + "]: " + ConsoleColors::RESET + m + "\n";
    }
    else
    {
        // Make the error message
        // pointer: points to the token where the error occured

        std::string pointer;
        std::string nextline = (tk.line < file.size()) ? file[tk.line] : "";
        std::string lastline = (tk.line > 1) ? file[tk.line - 2] + "\n" : "";

        size_t len = line.size();

        for (size_t i = 0; i < tk.col - 1 && i < len; i++)
        {
            pointer += (line[i] == '\t' ? "\t" : " ");
        }

        pointer += ConsoleColors::RED;

        for (size_t i = 0; i < tk.value.size(); i++)
            pointer += "^";

        pointer += ConsoleColors::RESET + "\n";

        return
            ConsoleColors::RED + "[" + n + "]: " + ConsoleColors::RESET + m + "\n\n"
            + "At " + tk.ctx->filename + ":" + std::to_string(tk.line) + ":" + std::to_string(tk.col) + "\n"
            + lastline
            + line + "\n"
            + pointer
            + nextline + "\n";
    }
}