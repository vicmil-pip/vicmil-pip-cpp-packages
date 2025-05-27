import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[0])) 
sys.path.append(str(Path(__file__).resolve().parents[1])) 
sys.path.append(str(Path(__file__).resolve().parents[2])) 
sys.path.append(str(Path(__file__).resolve().parents[3])) 
sys.path.append(str(Path(__file__).resolve().parents[4])) 

from vicmil_pip.packages.cppBuild import *
from vicmil_pip.packages.cppBindings.include.generate_bindings import CppBindingsGenerator

def build_lib():
    cpp_files = [get_directory_path(__file__, 0) + "/mylib.cpp"]

    build_setup = BuildSetup(browser=False)
    build_setup.add_default_parameters(
        cpp_file_paths=cpp_files, 
        output_dir=get_directory_path(__file__, 0) + "/bin",
    )

    build_setup.n9_output_file = get_directory_path(__file__) + "/bin/libmylib.so"

    build_setup.n5_additional_compiler_settings.append("-fPIC")
    build_setup.n5_additional_compiler_settings.append("-shared")
    build_setup.n4_macros.append("MYLIB_EXPORTS")

    build_setup.build_and_run()

def build_main():
    cpp_files = [get_directory_path(__file__, 0) + "/main.cpp"]

    build_setup = BuildSetup(browser=False)
    build_setup.add_default_parameters(
        cpp_file_paths=cpp_files, 
        output_dir=get_directory_path(__file__, 0) + "/bin",
    )

    build_setup.n7_library_paths.append(get_directory_path(__file__) + "/bin")
    build_setup.n8_library_files.append("mylib")

    build_setup.n5_additional_compiler_settings.append("-Wl,-rpath='$ORIGIN'")

    build_setup.build_and_run()


def generate_python_bindings():
    cpp_files = [get_directory_path(__file__, 0) + "/mylib.hpp", get_directory_path(__file__, 1) + "/src/bindings_util.hpp"]
    bindings_python_file = get_directory_path(__file__) + "/bin/mylib_bindings.py"

    bindings_generator = CppBindingsGenerator(cpp_files=cpp_files, output_python_file=bindings_python_file)

    bindings_generator.generate_bindings()


build_lib()
# build_main()
generate_python_bindings()


