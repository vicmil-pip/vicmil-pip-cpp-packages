# Contains information on how to build this package, what to include etc.

import sys
import platform
from pathlib import Path
sys.path.append(str(Path(__file__).resolve().parents[0])) 
sys.path.append(str(Path(__file__).resolve().parents[1])) 
sys.path.append(str(Path(__file__).resolve().parents[2])) 
sys.path.append(str(Path(__file__).resolve().parents[3])) 
sys.path.append(str(Path(__file__).resolve().parents[4])) 

from vicmil_pip.packages.cppBuild import BuildSetup, get_directory_path

def get_build_setup(browser: bool):
    new_build_setup = BuildSetup(browser=browser)
    new_build_setup.add_vicmil_pip_package("cppBasics")
    new_build_setup.add_vicmil_pip_package("cppGlm")

    new_build_setup.n6_include_paths.append(get_directory_path(__file__, 0))

    if not browser:
        platform_name = platform.system()

        if platform_name == "Windows": # Windows
            dependencies_directory = get_directory_path(__file__, 0) + "/sdl_opengl"

            # SDL
            new_build_setup.n6_include_paths.append(dependencies_directory + "/sdl_mingw/SDL2-2.30.7/x86_64-w64-mingw32/include/SDL2")
            new_build_setup.n6_include_paths.append(dependencies_directory + "/sdl_mingw/SDL2-2.30.7/x86_64-w64-mingw32/include")
            new_build_setup.n7_library_paths.append(dependencies_directory + "/sdl_mingw/SDL2-2.30.7/x86_64-w64-mingw32/lib")

            # SDL_image
            new_build_setup.n6_include_paths.append(dependencies_directory + "/sdl_mingw/SDL2_image-2.8.2/x86_64-w64-mingw32/include")
            new_build_setup.n7_library_paths.append(dependencies_directory + "/sdl_mingw/SDL2_image-2.8.2/x86_64-w64-mingw32/lib")

            # Glew
            new_build_setup.n6_include_paths.append(dependencies_directory + "/glew-win/include")
            new_build_setup.n7_library_paths.append(dependencies_directory + "/glew-win/lib/Release/x64")

            new_build_setup.n8_library_files.append("mingw32")
            new_build_setup.n8_library_files.append("glew32")
            new_build_setup.n8_library_files.append("opengl32")
            new_build_setup.n8_library_files.append("SDL2main")
            new_build_setup.n8_library_files.append("SDL2")
            new_build_setup.n8_library_files.append("SDL2_image")

        elif platform_name == "Linux": # Linux
            new_build_setup.n6_include_paths.append("/usr/include")

            new_build_setup.n8_library_files.append("GLEW")
            new_build_setup.n8_library_files.append("SDL2")
            new_build_setup.n8_library_files.append("SDL2_image")
            new_build_setup.n8_library_files.append("GL")  #(Used for OpenGL on desktops)

        else:
            raise NotImplementedError()

    else:
        new_build_setup.n5_additional_compiler_settings.append("-s USE_SDL=2")
        new_build_setup.n5_additional_compiler_settings.append("-s USE_SDL_IMAGE=2")
        new_build_setup.n5_additional_compiler_settings.append("-s FULL_ES3=1")

    return new_build_setup

import os
import shutil

def copy_all_files_with_extension(src_dir: str, dest_dir: str, file_extension: str):
    # Ensure the output directory exists
    os.makedirs(dest_dir, exist_ok=True)
    
    # Iterate over all files in the input directory
    for filename in os.listdir(src_dir):
        # Check if the file has the specified extension
        if filename.endswith(file_extension):
            print("Copy file: ", filename)
            # Construct full file paths
            source_file = os.path.join(src_dir, filename)
            destination_file = os.path.join(dest_dir, filename)
            
            # Copy the file
            shutil.copy2(source_file, destination_file)


def copy_dll_files(output_dir: str, browser: bool):
    print("Copy necessary dll files to the output directory")
    # Copy the necessary .dll or .so files to the output directory
    if not browser and platform.system() == "Windows": # Windows:
        dependencies_directory = get_directory_path(__file__, 0) + "/sdl_opengl"
        dll_paths = list()
        dll_paths.append(dependencies_directory + "/sdl_mingw/SDL2-2.30.7/x86_64-w64-mingw32/bin")
        dll_paths.append(dependencies_directory + "/sdl_mingw/SDL2_image-2.8.2/x86_64-w64-mingw32/bin")
        dll_paths.append(dependencies_directory + "/glew-win/bin/Release/x64")
        for lib_path in dll_paths:
            print("lib_path: ", lib_path)
            # Get all the .dll files and copy to the output dir
            copy_all_files_with_extension(src_dir=lib_path, dest_dir=output_dir, file_extension=".dll")