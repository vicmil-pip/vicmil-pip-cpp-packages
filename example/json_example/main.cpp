#include <iostream>
#include "vicmil_json.hpp"

int main()
{
    // create an empty structure (null)
    nlohmann::json j;

    // add a number stored as double (note the implicit conversion of j to an object)
    j["pi"] = 3.141;

    // add a Boolean stored as bool
    j["happy"] = true;

    // add a string stored as std::string
    j["name"] = "Niels";

    // add another null object by passing nullptr
    j["nothing"] = nullptr;

    // add an object inside the object
    j["answer"]["everything"] = 42;

    // add an array stored as std::vector (using an initializer list)
    j["list"] = {1, 0, 2};

    // add another object (using an initializer list of pairs)
    j["object"] = {{"currency", "USD"}, {"value", 42.99}};

    // serialization with pretty printing
    // pass in the amount of spaces to indent
    std::cout << j.dump(4) << std::endl;

    return 0;
}
