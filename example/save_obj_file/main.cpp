#include "util_std.hpp"
#include "util_obj_loader.hpp"
#include "util_glb_loader.hpp"
#include "util_stb.hpp"

int main()
{
    // std::string glb_file_name = "colored_cube2.glb";
    std::string glb_file_name = "textured_cube.glb";
    //  std::string glb_file_name = "minecraft_sheep.glb";

    std::cout << "Loading obj file" << std::endl;
    std::string data_path = vicmil::get_directory_path(vicmil::get_executable_path(), 1) + "/data";
    std::string glb_file_path = data_path + "/" + glb_file_name;
    std::cout << "glb file path: " << glb_file_path << std::endl;
    vicmil::GLBLoader glb_loader;
    glb_loader.load_from_file(glb_file_path);

    std::cout << glb_loader.get_metadata() << std::endl;

    vicmil::PrintModel(glb_loader.model);

    /*std::cout << "Save obj file" << std::endl;
    mesh.to_obj_file("output.obj", "");

    for (auto pair : mesh.embedded_images)
    {
        std::string file_path = pair.first;
        std::string ext = vicmil::get_file_extension(file_path);
        if (ext != "png")
        {
            file_path += ".png";
        }
        PrintExpr(file_path);
        vicmil::ImageRGBA_UChar_save_to_file(pair.second, file_path);
    }*/
    return 0;
}