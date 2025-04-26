#include "util_vicmil_gui.hpp"

vicmil::Window window;
vicmil::DefaultGpuPrograms gpu_programs;

std::shared_ptr<vicmil::WidgetData> widget_data;
vicmil::WidgetManager widget_manager;
std::shared_ptr<vicmil::ButtonWidget> button_widget;
std::shared_ptr<vicmil::SwitchWidget> switch_widget;

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
    button_widget = std::shared_ptr<vicmil::ButtonWidget>(new vicmil::ButtonWidget(widget_manager.data));
    switch_widget = std::shared_ptr<vicmil::SwitchWidget>(new vicmil::SwitchWidget(widget_manager.data));
    switch_widget.get()->attach_to(button_widget.get()->get_name(), vicmil::GuiEngine::attach_location::o_BottomRight_e_TopLeft);
    switch_widget.get()->set_size(50, 50);
    switch_widget.get()->set_layer(2);
    switch_widget.get()->set_enabled(true);
    Print(button_widget.get()->get_name());
    Print(switch_widget.get()->get_name());
    widget_manager.add_widget(button_widget);
    widget_manager.add_widget(switch_widget);

    vicmil::setup_fullscreen_canvas();
}

int main()
{
    vicmil::set_app_init(init);
    vicmil::set_app_update(update);
    vicmil::app_start();
    return 0;
}