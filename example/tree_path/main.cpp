#include "util_vicmil_gui.hpp"

int main(int argc, char *argv[])
{
    Print("Creating tree");
    vicmil::Tree my_tree;
    my_tree.add_tree_path({"file", "download"});
    my_tree.add_tree_path({"file", "upload"});
    my_tree.add_tree_path({"exit"});

    Print("\n" + my_tree.to_string());
    Print(vicmil::vec_to_str(my_tree.get_tree_path_children({"file"})));
    std::vector<std::vector<std::string>> all_tree_paths = my_tree.get_all_tree_paths();
    for (auto path : all_tree_paths)
    {
        Print(vicmil::vec_to_str(path));
    }

    return 0;
}