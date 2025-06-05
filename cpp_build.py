# Contains information on how to build this package, what to include etc.

import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[0])) 
sys.path.append(str(Path(__file__).resolve().parents[1])) 
sys.path.append(str(Path(__file__).resolve().parents[2])) 
sys.path.append(str(Path(__file__).resolve().parents[3])) 
sys.path.append(str(Path(__file__).resolve().parents[4])) 

from vicmil_pip.packages.cppBuild import BuildSetup, get_directory_path
import platform

def get_build_setup(browser: bool):
    new_build_setup = BuildSetup(browser=browser)
    if platform.system() == "Windows":
        # Avoid int64_t not defined
        new_build_setup.n5_additional_compiler_settings.append("-include stdint.h")
    
    return new_build_setup