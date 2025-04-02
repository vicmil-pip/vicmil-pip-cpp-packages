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

    if not browser:
        dependencies_directory = path_traverse_up(__file__, 0) + "/socket.io-client"
    
        new_build_setup.n2_cpp_files.append(dependencies_directory + "/src/sio_client.cpp")
        new_build_setup.n2_cpp_files.append(dependencies_directory + "/src/sio_socket.cpp")
        new_build_setup.n2_cpp_files.append(dependencies_directory + "/src/internal/sio_client_impl.cpp")
        new_build_setup.n2_cpp_files.append(dependencies_directory + "/src/internal/sio_packet.cpp")
        
        new_build_setup.n6_include_paths.append(f'-I "{dependencies_directory + "/lib"}"')
        new_build_setup.n6_include_paths.append(f'-I "{dependencies_directory + "/lib/websocketpp"}"')
        new_build_setup.n6_include_paths.append(f'-I "{dependencies_directory + "/lib/asio/asio/include"}"')
        new_build_setup.n6_include_paths.append(f'-I "{dependencies_directory + "/lib/rapidjson/include"}"')

        # These will force ASIO to compile without Boost
        new_build_setup.n4_macros.append("-DBOOST_DATE_TIME_NO_LIB")
        new_build_setup.n4_macros.append("-DBOOST_REGEX_NO_LIB")
        new_build_setup.n4_macros.append("-DASIO_STANDALONE")
            
        # These will force sioclient to compile with C++11
        new_build_setup.n4_macros.append("-D_WEBSOCKETPP_CPP11_STL_")
        new_build_setup.n4_macros.append("-D_WEBSOCKETPP_CPP11_FUNCTIONAL")
        new_build_setup.n4_macros.append("-D_WEBSOCKETPP_CPP11_TYPE_TRAITS_")
        new_build_setup.n4_macros.append("-D_WEBSOCKETPP_CPP11_CHRONO_")

        # Disable sockeio logging
        new_build_setup.n4_macros.append("-DSIO_DISABLE_LOGGING")

    return new_build_setup