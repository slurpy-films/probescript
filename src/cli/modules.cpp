#include "modules.hpp"

using namespace Probescript;

std::pair<std::unordered_map<std::string, fs::path>, Values::Val> ModuleIndexer::indexModules(fs::path fileName) {
    fs::path current = fs::is_directory(fileName) ? fileName : fs::current_path() / fileName.parent_path();
    fs::path projectFile;
    bool found = false;
    for (size_t i = 0; i < 10; ++i) {
        fs::path candidate = current / "project.json";

        if (fs::exists(candidate)) {
            projectFile = candidate;
            found = true;
            break;
        }

        if (current.has_parent_path()) {
            current = current.parent_path();
        } else {
            break;
        }
    }

    std::unordered_map<std::string, fs::path> modules;
    if (!found) return { modules, std::make_shared<Values::ObjectVal>() };

    for (const auto& entry : fs::recursive_directory_iterator(projectFile.parent_path())) {
        if (entry.is_regular_file()) {
            fs::path path = entry.path();

            if (path.extension() == ".probe" || path.extension() == ".prb") {
                std::ifstream file(path);

                if (file.is_open()) {
                    std::string firstLine;
                    std::getline(file, firstLine);

                    if (firstLine.find("module") == 0) {
                        std::string modulename;
                        if (firstLine.find(";") != std::string::npos) {
                            modulename = firstLine.substr(7, firstLine.find(";") - 7);
                        } else {
                            modulename = firstLine.substr(7);
                        }

                        modules[modulename] = path;
                    }
                }
            }
        }
    }

    std::ifstream stream(projectFile);
    std::string file((std::istreambuf_iterator<char>(stream)), std::istreambuf_iterator<char>());
    Stdlib::JSON::JSONParser parser(file);

    return { modules, parser.parse() };
}