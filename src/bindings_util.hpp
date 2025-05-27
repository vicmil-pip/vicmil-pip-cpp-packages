#pragma once

#include <vector>
#include <string>

#if defined _WIN32 || defined __CYGWIN__
#ifdef MYLIB_EXPORTS
#define MYLIB_API __declspec(dllexport)
#else
#define MYLIB_API __declspec(dllimport)
#endif
#else
#define MYLIB_API __attribute__((visibility("default")))
#endif

// Basic types bindings

extern "C"
{
    /* [vmcpp:export] { "type": "function", "name": "new_int_vector", "args": ["std::vector<int>** out_vec_pointer"], "return_type": "int" } [vmcpp:endexport] */
    MYLIB_API int new_int_vector(std::vector<int> **out_vec_pointer)
    {
        *out_vec_pointer = new std::vector<int>();
        return 0;
    }

    /* [vmcpp:export] { "type": "function", "name": "delete_int_vector", "args": ["std::vector<int>* in_vec"], "return_type": "int" } [vmcpp:endexport] */
    MYLIB_API int delete_int_vector(std::vector<int> *in_vec)
    {
        delete in_vec;
        return 0;
    }

    /* [vmcpp:export] { "type": "function", "name": "get_int_vector_length", "args": ["std::vector<int>* in_vec", "int* out_length"], "return_type": "int" } [vmcpp:endexport] */
    MYLIB_API int get_int_vector_length(std::vector<int> *in_vec, int *out_length)
    {
        *out_length = in_vec->size();
        return 0;
    }

    /* [vmcpp:export] { "type": "function", "name": "resize_int_vector", "args": ["std::vector<int>* in_vec", "int in_length"], "return_type": "int" } [vmcpp:endexport] */
    MYLIB_API int resize_int_vector(std::vector<int> *in_vec, int in_length)
    {
        in_vec->resize(in_length);
        return 0;
    }

    /* [vmcpp:export] { "type": "function", "name": "get_int_vector_array_ptr", "args": ["std::vector<int>* in_vec", "int** out_array_pointer"], "return_type": "int" } [vmcpp:endexport] */
    MYLIB_API int get_int_vector_array_ptr(std::vector<int> *in_vec, int **out_array_pointer)
    {
        *out_array_pointer = &(*in_vec)[0];
        return 0;
    }
}