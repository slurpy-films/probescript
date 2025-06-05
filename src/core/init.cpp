#include "init.hpp"

void Initializer::start(char* argv[])
{
    std::string name;
    std::string main;

    std::cout << "Probescript project initializer v1.0\n\n";
    std::cout << "Project name: ";
    std::getline(std::cin, name);

    std::cout << "\nMain file: (main.prb) ";
    std::getline(std::cin, main);
    if (main.empty()) main = "main.prb";

    fs::path projectFile(name + "/project.json");
    fs::path mainFile(name + "/" + main);
    fs::create_directory(name);
    
    std::ofstream outProjectFile(projectFile);
    std::ofstream outMainFile(mainFile);

    outMainFile << "probe Main {\n"
                << "\tMain() {\n"
                << "\t\tconsole.println(\"Hello World!\");\n"
                << "\t}\n"
                << "};";

    outProjectFile << "{\n\t\"name\": \"" << name << "\"" << ",\n\t\"main\": \"" << main << "\"\n}";

    outMainFile.close();
    outProjectFile.close();

    std::cout << "Project initialized! Run it with " << argv[0] << " run " << name;
}
