#include "application.hpp"

char __PROBESCRIPTVERSION__[] = "0.0.5";

int main(int argc, char* argv[])
{
    Application application(argc, argv);
    application.run();
}