import sys
import os
from pathlib import Path
import platform
sys.path.append(str(Path(__file__).resolve().parents[0])) 
sys.path.append(str(Path(__file__).resolve().parents[1])) 

import setup

cpp_compiler = setup.get_compiler_path()

cpp_source = setup.get_directory_path(__file__) + "/main.cpp"

executable_file = setup.get_directory_path(__file__) + "/bin/run" + setup.get_output_file_extension()

# Compile
setup.add_env_paths_to_compiler()
setup.run_command(f'"{cpp_compiler}" "{cpp_source}" -o "{executable_file}"')

if os.path.exists(executable_file):
    # Run executable that was built
    setup.run_command(f'"{executable_file}"')