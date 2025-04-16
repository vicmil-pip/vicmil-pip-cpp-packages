import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[0])) 
sys.path.append(str(Path(__file__).resolve().parents[1])) 
sys.path.append(str(Path(__file__).resolve().parents[2])) 
sys.path.append(str(Path(__file__).resolve().parents[3])) 
sys.path.append(str(Path(__file__).resolve().parents[4])) 
sys.path.append(str(Path(__file__).resolve().parents[5])) 

from vicmil_pip.packages.cppBuild import *

cpp_files = [path_traverse_up(__file__, 0) + "/main.cpp"]

build_setup = BuildSetup(browser=True)
build_setup.add_default_parameters(
    cpp_file_paths=cpp_files, 
    output_dir=path_traverse_up(__file__, 0) + "/bin",
)
build_setup.add_installed_vicmil_pip_packages()

build_setup.build_and_run()


