from setup import *

def get_compiler_path():
    if platform.system() == "Windows":
        return get_directory_path(__file__) + "/mingw64/bin/g++.exe"
    if platform.system() == "Linux":
        return get_directory_path(__file__) + "/clang/bin/clang"