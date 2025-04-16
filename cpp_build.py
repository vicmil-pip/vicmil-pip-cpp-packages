# Contains information on how to build this package, what to include etc.

import sys
import platform
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

    if not browser:
        platform_name = platform.system()

        if platform_name == "Windows": # Windows
            dependencies_directory = path_traverse_up(__file__, 0) + "/sdl_opengl"

            # SDL
            new_build_setup.n6_include_paths.append('-I"' + dependencies_directory + "/sdl_mingw/SDL2-2.30.7/x86_64-w64-mingw32/include/SDL2" + '"')
            new_build_setup.n6_include_paths.append('-I"' + dependencies_directory + "/sdl_mingw/SDL2-2.30.7/x86_64-w64-mingw32/include" + '"')
            new_build_setup.n7_library_paths.append('-L"' + dependencies_directory + "/sdl_mingw/SDL2-2.30.7/x86_64-w64-mingw32/lib" + '"')

            # SDL_image
            new_build_setup.n6_include_paths.append('-I"' + dependencies_directory + "/sdl_mingw/SDL2_image-2.8.2/x86_64-w64-mingw32/include" + '"')
            new_build_setup.n7_library_paths.append('-L"' + dependencies_directory + "/sdl_mingw/SDL2_image-2.8.2/x86_64-w64-mingw32/lib" + '"')

            # Glew
            new_build_setup.n6_include_paths.append('-I"' + dependencies_directory + "/glew-2.2.0/include" + '"')
            new_build_setup.n7_library_paths.append('-L"' + dependencies_directory + "/glew-2.2.0/lib/Release/x64" + '"')

            new_build_setup.n8_library_files.append('-l' + "mingw32")
            new_build_setup.n8_library_files.append('-l' + "glew32")
            new_build_setup.n8_library_files.append('-l' + "opengl32")
            new_build_setup.n8_library_files.append('-l' + "SDL2main")
            new_build_setup.n8_library_files.append('-l' + "SDL2")
            new_build_setup.n8_library_files.append('-l' + "SDL2_image")

        elif platform_name == "Linux": # Linux
            new_build_setup.n6_include_paths.append('-I' + "/usr/include")

            new_build_setup.n8_library_files.append('-l' + "GLEW")
            new_build_setup.n8_library_files.append('-l' + "SDL2")
            new_build_setup.n8_library_files.append('-l' + "SDL2_image")
            new_build_setup.n8_library_files.append('-l' + "GL")  #(Used for OpenGL on desktops)

        else:
            raise NotImplementedError()

    else:
        new_build_setup.n5_additional_compiler_settings.append("-s USE_SDL=2")
        new_build_setup.n5_additional_compiler_settings.append("-s USE_SDL_IMAGE=2")
        new_build_setup.n5_additional_compiler_settings.append("-s FULL_ES3=1")

    return new_build_setup