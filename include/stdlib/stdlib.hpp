#pragma once
#include "fs.hpp"
#include "date.hpp"
#include "runtime/values.hpp"
#include <string>
#include <unordered_map>
#include "random.hpp"
#include "http.hpp"
#include "json.hpp"

inline std::unordered_map<std::string, std::shared_ptr<ObjectVal>> getStdlib() {
    return {
        {"fs", std::make_shared<ObjectVal>(getFilesystemModule()) },
        {"date", std::make_shared<ObjectVal>(getDateModule()) },
        {"random", std::make_shared<ObjectVal>(createRandomModule()) },
        {"http", std::make_shared<ObjectVal>(getHttpModule()) },
        {"json", std::make_shared<ObjectVal>(getJsonModule())}
    };
};