#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <filesystem>
#include <cstdlib>
#include <regex>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

#ifdef _WIN32
void addToPathWindows(const std::string& path)
{
    HKEY hKey;
    const char* subkey = "Environment";
    LONG result = RegOpenKeyExA(HKEY_CURRENT_USER, subkey, 0, KEY_READ | KEY_WRITE, &hKey);
    if (result != ERROR_SUCCESS)
    {
        std::cerr << "Failed to open registry key.\n";
        return;
    }

    char oldPath[32767];
    DWORD pathLen = sizeof(oldPath);
    result = RegQueryValueExA(hKey, "Path", nullptr, nullptr, (LPBYTE)&oldPath, &pathLen);
    std::string newPath;

    if (result == ERROR_SUCCESS)
    {
        std::string currentPath = oldPath;
        if (currentPath.find(path) == std::string::npos)
        {
            newPath = currentPath + ";" + path;
        } else
        {
            std::cout << "Path already contains probescript location.\n";
            RegCloseKey(hKey);
            return;
        }
    } else {
        newPath = path;
    }

    result = RegSetValueExA(hKey, "Path", 0, REG_EXPAND_SZ, (const BYTE*)newPath.c_str(), newPath.size() + 1);
    if (result == ERROR_SUCCESS)
    {
        std::cout << "Path updated. You may need to restart your session.\n";
    } else
    {
        std::cerr << "Failed to update PATH.\n";
    }

    RegCloseKey(hKey);
}
#endif

int main()
{
    std::cout << "Welcome to the probescript installer! This program will take you through the process of installing probescript on your computer\n\n";
    std::this_thread::sleep_for(std::chrono::seconds(3));

    std::string binaryPath;

    std::string localBinary =
#ifdef _WIN32
        "probescript.exe";
#else
        "probescript";
#endif
    std::string exePath = (fs::current_path() / localBinary).string();

    if (fs::exists(exePath))
    {
        binaryPath = exePath;
    } else
    {
        std::cout << "Enter path to your probescript binary (leave blank to fetch):\n> ";
        std::getline(std::cin, binaryPath);

        if (binaryPath.empty())
        {
            std::cerr << "No path entered. Exiting.\n";
            return 1;
        } else if (!fs::exists(binaryPath))
        {
            std::cerr << "\nError: File does not exist.\n";
            return 1;
        }
    }

#ifdef _WIN32
    std::string targetDir;
    std::cout << "Enter probescript installation directory (default: C:\\Program Files\\Probescript):\n>";
    std::getline(std::cin, targetDir);

    if (targetDir.empty()) targetDir = "C:\\Program Files\\Probescript";
    try
    {
        fs::create_directories(targetDir);
    } catch (...)
    {
        std::cout << "Failed to create program files directory. This probably means that you are not running this program with administrative permissions or you already have a folder named 'Probescript' in your Program Files.\n";
        std::cout << "Press Enter to exit...\n";
        std::cin.get();
        
        exit(1);
    }

    std::string targetPath = targetDir + "\\probescript.exe";

    try
    {
        fs::copy(binaryPath, targetPath, fs::copy_options::overwrite_existing);
        std::cout << "\nCopied to: " << targetPath << "\n";
        bool addToPath;
        bool done = false;
        while (!done)
        {
            std::cout << "\nAdd probescript to path? [y/n]\n >";
            std::string input;
            std::cin >> input;
            if (input != "y" && input != "n") continue;
            addToPath = (input == "y");
            done = true;
        }
        if (addToPath) addToPathWindows(targetDir);
    } catch (const std::exception& e)
    {
        std::cerr << "Failed to copy binary: " << e.what() << "\n";
        std::cout << "Press Enter to exit...\n";
        std::cin.get();
        return 1;
    }

#else
    std::cout << "\nInstall binary to /usr/local/bin? [Y/n]: ";
    std::string choice;
    std::getline(std::cin, choice);
    if (choice.empty() || choice == "Y" || choice == "y")
    {
        std::string cmd = "sudo cp \"" + binaryPath + "\" /usr/local/bin/probescript && sudo chmod +x /usr/local/bin/probescript";
        int result = std::system(cmd.c_str());
        if (result == 0)
        {
            std::cout << "\nProbescript successfully installed to /usr/local/bin.\n";
            std::cout << "You can now run it from anywhere using `probescript`\n";
        } else
        {
            std::cerr << "\nFailed to install binary. Are you running with sudo/root access?\n";
            std::cout << "Press Enter to exit...\n";
            std::cin.get();
        }
    } else
    {
        std::cout << "Skipping system installation. You can manually run it from:\n" << binaryPath << "\n";
    }
#endif

    std::cout << "\033[36m" << R"DELIM(
  ________  ________  ________  ________  _______   ________  ________  ________  ___  ________  _________   
|\   __  \|\   __  \ \   __  \|\   __  \|\  ___ \ |\   ____\|\   ____\|\   __  \|\  \|\   __  \|\___   ___\
\ \  \|\  \ \  \|\  \ \  \|\  \ \  \|\ /\ \   __/ \ \  \___|\ \  \___|\ \  \|\  \ \  \ \  \|\  \|___ \  \_| 
 \ \   ____\ \   _  _\ \  \\\  \ \   __  \ \  \   _\ \_____  \ \  \    \ \   _  _\ \  \ \   ____\   \ \  \
  \ \  \___|\ \  \\  \\ \  \\\  \ \  \|\  \ \  \_|\ \|____|\  \ \  \____\ \  \\  \\ \  \ \  \___|    \ \  \
   \ \__\    \ \__\\ _\\ \_______\ \_______\ \_______\____\_\  \ \_______\ \__\\ _\\ \__\ \__\        \ \__\
    \|__|     \|__|\|__|\|_______|\|_______|\|_______|\_________\|_______|\|__|\|__|\|__|\|__|         \|__|
                                                     \|_________|                                                                                                    
    )DELIM" << "\033[0m";

    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "Welcome to probescript! To see a list of commands you can use with probescript, run the 'probescript help' command.\nIf you are interested in contributing to probescript, you can check out the repository at https://github.com/slurpy-films/probescript.\n";
    std::cout << "Press Enter to exit...";
    std::cin.get();
    return 0;
}
