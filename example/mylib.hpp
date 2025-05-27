#pragma once

#include "../src/bindings_util.hpp"

extern "C"
{
    /* [vmcpp:export] { "type": "function", "name": "say_hello", "args": ["const char* name"], "return_type": "void" } [vmcpp:endexport] */
    MYLIB_API void say_hello(const char *name);
}
