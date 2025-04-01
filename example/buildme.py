import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[0])) 
sys.path.append(str(Path(__file__).resolve().parents[1])) 
sys.path.append(str(Path(__file__).resolve().parents[2])) 
sys.path.append(str(Path(__file__).resolve().parents[3])) 
sys.path.append(str(Path(__file__).resolve().parents[4])) 

from vicmil_pip.packages.cppBuild import *

cpp_files = [path_traverse_up(__file__, 0) + "/main.cpp"]

build_setup = BuildSetup(browser=False)
build_setup.add_default_parameters(
    cpp_file_paths=cpp_files, 
    output_dir=path_traverse_up(__file__, 0) + "/bin",
)

build_setup.build_and_run()


