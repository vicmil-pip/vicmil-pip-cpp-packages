from typing import List
import platform
import pathlib
import os
import sys
from pathlib import Path
import importlib.util

sys.path.append(str(Path(__file__).resolve().parents[1])) 
sys.path.append(str(Path(__file__).resolve().parents[2])) 


"""
    Used for building projects in c++
    The build setup is a specification for how to build a c++ project,
    you can merge one build setup with another build setup, to form a combined build setup,
    this way you can merge different libraries build setup. 
    
    Since it is python, you do not need additional dependencies such as cmake, and it is 
    really easy to run projects, just run the python file :)
"""
class BuildSetup:
    def __init__(self, browser: bool):
        # When building c++ projects, this is in general the order the flags should be
        self.n1_compiler_path: str = None
        self.n2_cpp_files: list = []
        self.n3_optimization_level: list = []
        self.n4_macros: list = []
        self.n5_additional_compiler_settings: list = []
        self.n6_include_paths: list = []
        self.n7_library_paths: list = []
        self.n8_library_files: list = []
        self.n9_output_file: str = None

        self.browser_flag = browser

        
    def add_default_parameters(self, cpp_file_paths: List[str], output_dir: str, O2_optimization=True):
        # Set compiler path, and output path
        self.n1_compiler_path: str = get_default_compiler_path(browser=self.browser_flag)
        self.n9_output_file: str = output_dir + "/" + get_default_output_file(browser=self.browser_flag)


        # Add other paramters
        new_build_setup = BuildSetup(self.browser_flag)
        new_build_setup.n2_cpp_files =                      [path_ for path_ in cpp_file_paths]
        new_build_setup.n3_optimization_level =             ["-std=c++11"] # Specify using c++11 by default
        new_build_setup.n4_macros =                         []
        new_build_setup.n5_additional_compiler_settings =   []
        new_build_setup.n6_include_paths =                  ["-I \"" + path_traverse_up(__file__, 0) + "\""]
        new_build_setup.n7_library_paths =                  []
        new_build_setup.n8_library_files =                  []

        # Add conditional parameters
        if O2_optimization:
            new_build_setup.n3_optimization_level.append("-O2") # Add some default optimization during compilation

        if self.browser_flag:
            # Add additional flags when compiling with emscripten
            new_build_setup.n5_additional_compiler_settings.append("-s ASYNCIFY=1") # Enable sleep with emscripten
            new_build_setup.n5_additional_compiler_settings.append("-s ALLOW_MEMORY_GROWTH") # Do not limit the app to a small amount of memory
            new_build_setup.n5_additional_compiler_settings.append("-s EXPORTED_RUNTIME_METHODS=ccall,cwrap")
            new_build_setup.n8_library_files.append("-lembind") # Allow binding javascript functions from c++

        self.add_other_build_setup(new_build_setup)


    def add_vicmil_pip_package(self, package_name: str):
        package_path = path_traverse_up(__file__, 1) + "/" + package_name
        if os.path.exists(package_path + "/cpp_build.py"):
            print(f"including {package_name} cpp_build config")

            # Load the file
            spec = importlib.util.spec_from_file_location(package_name, package_path + "/cpp_build.py")
            module = importlib.util.module_from_spec(spec)
            spec.loader.exec_module(module)

            # Load the config from the file
            new_build_setup: BuildSetup = module.get_build_setup(self.browser_flag)  # Call a function from the script

            # Add config to build setup
            self.add_other_build_setup(new_build_setup)


    def add_installed_vicmil_pip_packages(self):
        # Iterate through all installed vicmil pip packages
        # For each package:
        #   Get their default build setup, if there is one
        #   Append paramters to this build setup
        # List all the packages and print their description
        dirs = os.listdir(path_traverse_up(__file__, 1))        
        folders = list()
        for f in dirs:
            if f == "__pycache__":
                continue
            if f == "venv":
                continue
            folders.append(f)

        print(f"found {len(folders)} installed packages")
        print(folders)
        for package_name in folders:
            self.add_vicmil_pip_package(package_name)


    def add_other_build_setup(self, other):
        other: BuildSetup = other # Specify type

        for arg_ in other.n2_cpp_files:
            if not arg_ in self.n2_cpp_files:
                self.n2_cpp_files.append(arg_)

        for arg_ in other.n3_optimization_level:
            if not arg_ in self.n3_optimization_level:
                self.n3_optimization_level.append(arg_)

        for arg_ in other.n4_macros:
            if not arg_ in self.n4_macros:
                self.n4_macros.append(arg_)

        for arg_ in other.n5_additional_compiler_settings:
            if not arg_ in self.n5_additional_compiler_settings:
                self.n5_additional_compiler_settings.append(arg_)

        for arg_ in other.n6_include_paths:
            if not arg_ in self.n6_include_paths:
                self.n6_include_paths.append(arg_)

        for arg_ in other.n7_library_paths:
            if not arg_ in self.n7_library_paths:
                self.n7_library_paths.append(arg_)

        for arg_ in other.n8_library_files:
            if not arg_ in self.n8_library_files:
                self.n8_library_files.append(arg_)


    def generate_build_command(self):
        arguments = [
            self.n1_compiler_path, 
            " ".join(['"' + path_ + '"' for path_ in self.n2_cpp_files]),
            " ".join(self.n3_optimization_level),
            " ".join(self.n4_macros),
            " ".join(self.n5_additional_compiler_settings),
            " ".join(self.n6_include_paths),
            " ".join(self.n7_library_paths),
            " ".join(self.n8_library_files),
            "-o " + '"' + self.n9_output_file + '"',
        ]

        # Remove arguments with length 0
        arguments = filter(lambda arg: len(arg) > 0, arguments)

        return " ".join(arguments)
    
    def build(self):
        build_command = self.generate_build_command()

        # Remove the output file if it exists already
        if os.path.exists(self.n9_output_file):
            os.remove(self.n9_output_file)

        if not os.path.exists(path_traverse_up(self.n9_output_file, 0)):
            # Create the output directory if it does not exist
            os.makedirs(path_traverse_up(self.n9_output_file, 0), exist_ok=True)

        # Run the build command
        print(build_command)
        run_command(build_command)

    def run(self):
        invoke_file(self.n9_output_file)

    def build_and_run(self):
        self.build()
        self.run()

        

def path_traverse_up(path: str, levels_up: int) -> str:
    """Traverse the provided path upwards

    Parameters
    ----------
        path (str): The path to start from, tips: use __file__ to get path of the current file
        levels_up (int): The number of directories to go upwards

    Returns
    -------
        str: The path after the traversal, eg "/some/file/path"
    """

    parents = pathlib.Path(path).parents
    path_raw = str(parents[levels_up].resolve())
    return path_raw.replace("\\", "/")


def get_default_output_file(browser = False):
    platform_name = platform.system()

    if not browser:
        if platform_name == "Windows": # Windows
            return "run.exe"

        elif platform_name == "Linux": # Linux
            return "run.out"

        else:
            raise NotImplementedError()
        
    else:
        return "run.html"


def run_command(command: str) -> None:
    """Run a command in the terminal"""
    platform_name = platform.system()
    if platform_name == "Windows": # Windows
        print("running command: ", f'powershell; &{command}')
        if command[0] != '"':
            os.system(f'powershell; {command}')
        else:
            os.system(f'powershell; &{command}')
    else:
        os.system(command)


def invoke_file(file_path: str):
    if not os.path.exists(file_path):
        print(file_path + " does not exist")
        return

    file_extension = file_path.split(".")[-1]

    if file_extension == "html":
        # Create a local python server and open the file in the browser
        launch_html_page(file_path)

    elif file_extension == "exe" or file_extension == "out":
        # Navigate to where the file is located and invoke the file
        file_directory = path_traverse_up(file_path, 0)
        os.chdir(file_directory) # Change active directory
        run_command('"' + file_path + '"')


def launch_html_page(html_file_path: str):
    import webbrowser
    """ Start the webbrowser if not already open and launch the html page

    Parameters
    ----------
        html_file_path (str): The path to the html file that should be shown in the browser

    Returns
    -------
        None
    """
    os.chdir(path_traverse_up(html_file_path, levels_up=0))
    if not (os.path.exists(html_file_path)):
        print("html file does not exist!")
        return
    
    file_name: str = html_file_path.replace("\\", "/").rsplit("/", maxsplit=1)[-1]
    webbrowser.open("http://localhost:8000/" + file_name, new=0, autoraise=True)

    try:
        run_command("python3 -m http.server --bind localhost")
    except Exception as e:
        pass

    run_command("python -m http.server --bind localhost")


# Get the defualt compiler path within vicmil lib
def get_default_compiler_path(browser = False):
    if not browser:
        if os.path.exists(path_traverse_up(__file__, 1) + "/cppBasicCompiler"):
            import packages.cppBasicCompiler.setup as compiler_setup
            compiler_setup.add_env_paths_to_compiler()
            return compiler_setup.get_compiler_path()
        else:
            return "g++"

    else:
        if platform.system() == "Windows": # Windows
            return '"' + path_traverse_up(__file__, 1) + "/cppEmsdk/emsdk/upstream/emscripten/em++.bat" + '"'
        else:
            return '"' + path_traverse_up(__file__, 1) + "/cppEmsdk/emsdk/upstream/emscripten/em++" + '"'

    
def convert_file_to_header(input_file: str, output_header: str=None):
    if not output_header:
        output_header = input_file + ".hpp"
    # Check if input file exists
    if not os.path.exists(input_file):
        print(f"Error: The file '{input_file}' does not exist.")
        sys.exit(1)

    # Open the input file for reading
    with open(input_file, 'rb') as f:
        file_data = f.read()

    var_name = output_header.split("/")[-1].upper().replace('.', '_').replace('/', '_').replace('\\', '_').replace('-', '_')
    # Start creating the header file
    with open(output_header, 'w') as header_file:
        # Write the C++ array header
        header_file.write(f"#ifndef {var_name}_H\n")
        header_file.write(f"#define {var_name}_H\n\n")
        header_file.write(f"unsigned char {var_name}_data[] = {{\n")

        # Write the contents of the file as a C++ array
        for i in range(0, len(file_data), 12):  # 12 bytes per line (adjust as needed)
            # Format each line with the appropriate number of bytes
            header_file.write("    " + ", ".join(f"0x{byte:02X}" for byte in file_data[i:i+12]))
            header_file.write(",\n")

        # End the array
        header_file.write("};\n\n")
        header_file.write(f"unsigned int {var_name}_size = {len(file_data)};\n")
        header_file.write(f"#endif // {var_name}_H\n")

    print(f"Header file '{output_header}' has been created successfully.")


import subprocess
def invoke_python_file_using_subprocess(python_env_path: str, file_path: str, logfile_path: str = None) -> subprocess.Popen:
    if not os.path.exists(python_env_path):
        print(f"invalid path: {python_env_path}")

    if not os.path.exists(file_path):
        print(f"invalid path: {file_path}")

    current_directory = str(pathlib.Path(file_path).parents[0].resolve()).replace("\\", "/")
    os.chdir(current_directory) # Set active directory to the current directory

    command = ""
    my_os = platform.system()
    if logfile_path:
        if my_os == "Windows":
            command = f'powershell; &"{python_env_path}/Scripts/python" -u "{file_path}" > "{logfile_path}"'
        else:
            command = f'"{python_env_path}/bin/python" -u "{file_path}" > "{logfile_path}"'
    else:
        if my_os == "Windows":
            command = f'powershell; &"{python_env_path}/Scripts/python" -u "{file_path}"'
        else:
            command = f'"{python_env_path}/bin/python" -u "{file_path}"'

    new_process = subprocess.Popen(command, shell=True)
    return new_process
