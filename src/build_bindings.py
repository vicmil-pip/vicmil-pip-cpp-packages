import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[0])) 
sys.path.append(str(Path(__file__).resolve().parents[1])) 
sys.path.append(str(Path(__file__).resolve().parents[2])) 
sys.path.append(str(Path(__file__).resolve().parents[3])) 
sys.path.append(str(Path(__file__).resolve().parents[4])) 

from vicmil_pip.packages.pyUtil import *
from vicmil_pip.packages.cppBindings.include.generate_bindings import CppBindingsGenerator


def generate_python_bindings():
    cpp_files = [get_directory_path(__file__, 0) + "/bindings_util.hpp"]
    bindings_python_file = get_directory_path(__file__) + "/bin/bindings_util_bindings.py"

    bindings_generator = CppBindingsGenerator(cpp_files=cpp_files, output_python_file=bindings_python_file)

    bindings_generator.generate_bindings()


generate_python_bindings()


