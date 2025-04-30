---
title: /__init__.py
source_file: /__init__.py
description: 
generated_from: vmdoc
source_code_file: _init_.py_b76991.txt
---

[📄 View raw source code](_init_.py_b76991.txt)

# BuildSetup

Used for building projects in c++
The build setup is a specification for how to build a c++ project,
you can merge one build setup with another build setup, to form a combined build setup,
this way you can merge different libraries build setup. 

Since it is python, you do not need additional dependencies such as cmake, and it is 
really easy to run projects, just run the python file :)

## Usage

The basic idea is just to allow adding compiler arguments, where they are later put in an order so they compile correctly
The different types of arguments are the following, where the n# symbolizes in which order they will appear in the build command

* n1_compiler_path: str # Which compiler to use, defualts to g++ if browser=false, otherwise em++
* n2_cpp_files: List[str] # Paths to cpp files, i.e. ["/some/path/main.cpp", "some/path/other.cpp"]
* n3_optimization_level: List[str] # Additional optimization flags, can also be used to specify c++ version
* n4_macros: List[str] # Macros to include, will automatically get a -D flag
* n5_additional_compiler_settings: List[str] # Additional compiler settings, you need to specify the flag
* n6_include_paths: List[str] # Include paths, will automatically get a -I flag
* n7_library_paths: List[str] # Library paths, will automatically get a -L flag
* n8_library_files: List[str] # Library files, will automatically get a -l flag
* n9_output_file: str # Path to the file where the binary will go, typically bin/run.exe or bin/run.out or bin/run.html depending on configuration

Finally you can also specify if you want the system to build for the browser or not(i.e. compile using emscripten)

- browser_flag: bool

