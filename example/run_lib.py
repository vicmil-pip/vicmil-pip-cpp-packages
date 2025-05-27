"""
Simple example for how to load a c++ library into python and run it
"""

import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[0])) 
sys.path.append(str(Path(__file__).resolve().parents[1])) 
sys.path.append(str(Path(__file__).resolve().parents[2])) 
sys.path.append(str(Path(__file__).resolve().parents[3])) 
sys.path.append(str(Path(__file__).resolve().parents[4])) 

from vicmil_pip.packages.cppBuild import *
from vicmil_pip.packages.cppBindings.include.basic_bindings import *
from example.bin.mylib_bindings import CppBindings

loaded_library = LoadedLibrary(get_directory_path(__file__) + "/bin/libmylib")
bindings = CppBindings()
bindings.loaded_library = loaded_library

arg = CppCharPtr("Mr. Python")
bindings.say_hello(name = arg, allocated_object=False)

my_vec = CppIntVector(bindings, [1, 2, 3, 4])
print(len(my_vec))

my_vec.delete()

bindings.loaded_library.cleanup()