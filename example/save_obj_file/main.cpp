#include "util_std.hpp"
#include "util_obj_loader.hpp"

int main()
{
    // std::string obj_file_name = "colored_cube.obj";
    std::string obj_file_name = "textured_cube.obj";
    // std::string obj_file_name = "minecraft_sheep.obj";

    std::cout << "Loading obj file" << std::endl;
    std::string data_path = vicmil::get_directory_path(vicmil::get_executable_path(), 1) + "/data";
    std::string obj_file_path = data_path + "/" + obj_file_name;
    std::cout << "obj file path: " << obj_file_path << std::endl;
    vicmil::Mesh mesh = vicmil::load_obj_file(obj_file_path, data_path);

    std::cout << mesh.get_metadata() << std::endl;

    std::cout << "Save obj file" << std::endl;
    mesh.to_obj_file("output.obj", "");
    return 0;
}