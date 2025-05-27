"""
[vmdoc:description]
Automatically generate .md files using tags inside of files
[vmdoc:enddescription]
"""

import sys
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[0]))
sys.path.append(str(Path(__file__).resolve().parents[1]))
sys.path.append(str(Path(__file__).resolve().parents[2]))
sys.path.append(str(Path(__file__).resolve().parents[3]))
sys.path.append(str(Path(__file__).resolve().parents[4]))
sys.path.append(str(Path(__file__).resolve().parents[5]))
from typing import List
import json

from vicmil_pip.packages.pyUtil import *

def get_func_bindings(tag_dict):
    if not "name" in tag_dict.keys():
        print("warning: missing name argument: ", tag_dict)
        return ""
    
    func_name = tag_dict["name"]

    func_args = list()
    if not "args" in tag_dict.keys():
        print("warning: missing args argument: ", tag_dict)
        return ""
    
    if not "return_type" in tag_dict.keys():
        print("warning: missing return_type argument: ", tag_dict)
        return ""

    for argument in tag_dict["args"]:
        argument: str = argument
        argument_type, argument_name = argument.rsplit(" ", 1)
        argument_name = argument_name.strip()
        argument_type = argument_type.strip()
        func_args.append((argument_name, argument_type))

    indent = " " * 8

    func_content = f"""
    def {func_name}(self, {", ".join([func_arg[0] + ": CppArgument" for func_arg in func_args])}, allocated_object=True):
        '''
        input:
{("\n").join([indent + "  - " + func_arg[0] + ": " + func_arg[1] for func_arg in func_args])}

        output: {tag_dict["return_type"]}
        '''
{"\n".join([indent + "assert " + func_arg[0] + ".argument_type == " + '"' + func_arg[1] + '"' for func_arg in func_args])}

        func_output = CppArgument()
        func_output.argument_type = "{tag_dict["return_type"]}"
        func_output.argument_name = "{tag_dict["name"]}" # Inherit the function name, used to determine where the object was allocated for debugging purposes

        func_output._argument_value = self.loaded_library.loaded_library.{func_name}({", ".join([func_arg[0] + "._cpp_object" for func_arg in func_args])})

        if allocated_object:
            func_output.allocation_id = self.loaded_library.register_allocation("{tag_dict["name"]}")

        return func_output"""
    
    return func_content

class CppBindingsGenerator:
    def __init__(self, cpp_files: List[str], output_python_file: str):
        self.cpp_files = cpp_files
        self.output_python_file = output_python_file

    def generate_bindings(self):
        # Extract the tags contents from all the cpp files
        all_files_tags_contents = list()
        for file_path in self.cpp_files:
            tag_contents = get_docs_tag_contents(start_tag="[vmcpp:export]", end_tag="[vmcpp:endexport]", file_path=file_path)
            all_files_tags_contents += tag_contents

        output_file_contents = """
from typing import Any

class CppArgument:
    def __init__(self):
        self.argument_type: str = None
        self.argument_name: str = None
        self._cpp_object: Any = None
        self._other_args: Any = None
        self.need_destructor = True

class CppBindings:
    def __init__(self):
        self.loaded_library = None""" + "\n\n"

        for tag_content in all_files_tags_contents:
            tag_dict: dict = json.loads(tag_content)
            if not "type" in tag_dict.keys():
                print("warning: missing type argument: ", tag_content)
                continue

            if tag_dict["type"] == "function":
                output_file_contents += get_func_bindings(tag_dict=tag_dict) + "\n\n"

        os.makedirs(get_directory_path(self.output_python_file), exist_ok=True)
        with open(self.output_python_file, "w+") as file:
            file.write(output_file_contents)

                





