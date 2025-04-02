# Contains information on how to build this package, what to include etc.

import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[0])) 
sys.path.append(str(Path(__file__).resolve().parents[1])) 
sys.path.append(str(Path(__file__).resolve().parents[2])) 
sys.path.append(str(Path(__file__).resolve().parents[3])) 
sys.path.append(str(Path(__file__).resolve().parents[4])) 

from vicmil_pip.packages.cppBuild import BuildSetup, path_traverse_up

def get_build_setup(browser: bool):
    new_build_setup = BuildSetup(browser=browser)
    new_build_setup.n6_include_paths.append(f'-I "{path_traverse_up(__file__, 0)}"')

    # Include smol-atlas
    new_build_setup.n2_cpp_files.append(path_traverse_up(__file__, 0) + "/smol-atlas/src/smol-atlas.cpp")
    new_build_setup.n6_include_paths.append(f'-I "{path_traverse_up(__file__, 0) + "/smol-atlas/src/"}"')
    
    return new_build_setup