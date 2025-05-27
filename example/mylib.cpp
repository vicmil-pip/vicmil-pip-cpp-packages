#include "mylib.hpp"
#include <iostream>

void say_hello(const char *name)
{
    std::cout << "Hello, " << name << "! From a cross-platform shared library." << std::endl;
}
