#include "util_vicmil_gui_bonus.hpp"
#include "Noto Sans Mono CJK JP Regular.otf.hpp"

vicmil::Window window;
vicmil::DefaultGpuPrograms gpu_programs;

vicmil::WidgetManager widget_manager;
std::shared_ptr<vicmil::DropDownWidget> drop_down_widget;

void update()
{
    std::vector<SDL_Event> events = vicmil::update_SDL();
    vicmil::clear_screen();
    widget_manager.update(window, events);
    widget_manager.draw(gpu_programs);
    window.show_on_screen();
}

void init()
{
    // Init SDL and Window
    vicmil::init_SDL();
    window = vicmil::Window(512, 512, "Hello rectangle minimal");

    gpu_programs = vicmil::DefaultGpuPrograms();
    gpu_programs.init_default_gpu_programs();

    // Setup widgets
    widget_manager = vicmil::WidgetManager();
    drop_down_widget = std::shared_ptr<vicmil::DropDownWidget>(new vicmil::DropDownWidget(widget_manager.data));
    drop_down_widget.get()->set_size(50, 20);
    widget_manager.add_widget(drop_down_widget);

    drop_down_widget->dropdown_tree.add_tree_path({"file", "download", "obj"});
    drop_down_widget->dropdown_tree.add_tree_path({"file", "download", "zip"});
    drop_down_widget->dropdown_tree.add_tree_path({"file", "upload"});
    drop_down_widget->dropdown_tree.add_tree_path({"exit"});

    // Load fonts
    widget_manager.load_font(get_NOTO_SANS_MONO_CJK_JP_REGULAR_OTF_data(), get_NOTO_SANS_MONO_CJK_JP_REGULAR_OTF_size());

    vicmil::setup_fullscreen_canvas();
}

int main()
{
    vicmil::set_app_init(init);
    vicmil::set_app_update(update);
    vicmil::app_start();
    return 0;
}