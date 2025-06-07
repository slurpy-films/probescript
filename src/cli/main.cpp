#include "application.hpp"

char __PROBESCRIPTVERSION__[] = "1.0";

int main(int argc, char* argv[])
{
    Application application(argc, argv);
    application.run();
}