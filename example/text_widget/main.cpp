#include "util_vicmil_gui_bonus.hpp"
#include "Noto Sans Mono CJK JP Regular.otf.hpp"

vicmil::Window window;
vicmil::DefaultGpuPrograms gpu_programs;

vicmil::WidgetManager widget_manager;
std::shared_ptr<vicmil::TextBoxWidget> text_box_widget;

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
    text_box_widget = std::shared_ptr<vicmil::TextBoxWidget>(new vicmil::TextBoxWidget(widget_manager.data));
    text_box_widget.get()->set_size(200, 200);
    widget_manager.add_widget(text_box_widget);

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