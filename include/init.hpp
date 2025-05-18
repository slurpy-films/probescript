#pragma once
#include <filesystem>
#include <iostream>
#include <string>
#include <fstream>

namespace fs = std::filesystem;

class Initializer
{
public:
    void start(char* argv[]);
};