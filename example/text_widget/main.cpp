#include "util_vicmil_gui_bonus.hpp"
#include "Noto Sans Mono CJK JP Regular.otf.hpp"

vicmil::Window window;
vicmil::DefaultGpuPrograms gpu_programs;

vicmil::WidgetManager widget_manager;
std::shared_ptr<vicmil::TextBoxWidget> text_box_widget;

bool saved_as_png = false;

void update()
{
    std::vector<SDL_Event> events = vicmil::update_SDL();
    vicmil::clear_screen();
    widget_manager.update(window, events);
    widget_manager.draw(gpu_programs);
    // vicmil::clear_screen();
    // std::vector<vicmil::CoordTexCoord_XYZUV_f> vertices = {};
    // vicmil::add_texture_rect_to_triangle_buffer(vertices, vicmil::GuiEngine::RectGL(-1.0, 1.0, 2.0, 2.0), 1);

    // if (!saved_as_png)
    //{
    //     vicmil::ImageRGBA_UChar_save_as_png(widget_manager.data->font_image_manager->image_manager->cpu_image, "my_image.png");
    //     saved_as_png = true;
    // }

    // widget_manager.data->image_manager->gpu_image.texture.bind();
    // gpu_programs.draw_2d_CoordTexCoord_XYZUV_f_vertex_buffer(vertices, widget_manager.data->image_manager->gpu_image);
    window.show_on_screen();
}

void init()
{
    // Init SDL and Window
    vicmil::init_SDL();
#if defined(__EMSCRIPTEN__)
    window = vicmil::Window(2048, 2048, "Hello rectangle minimal");
#else
    window = vicmil::Window(512, 512, "Hello rectangle minimal");
#endif

    gpu_programs = vicmil::DefaultGpuPrograms();
    gpu_programs.init_default_gpu_programs();

    // Setup widgets
    widget_manager = vicmil::WidgetManager();
    widget_manager.data->gui_engine->set_screen_size(512, 512);

    // Load fonts
    widget_manager.load_font(get_NOTO_SANS_MONO_CJK_JP_REGULAR_OTF_data(), get_NOTO_SANS_MONO_CJK_JP_REGULAR_OTF_size());

    text_box_widget = std::shared_ptr<vicmil::TextBoxWidget>(new vicmil::TextBoxWidget(widget_manager.data));
    text_box_widget.get()->set_size(200, 200);
    text_box_widget.get()->text_input.set_text("text");
    widget_manager.add_widget(text_box_widget);
    text_box_widget.get()->update();
    widget_manager.data->gui_engine->build();

    vicmil::setup_fullscreen_canvas();
}

int main(int argc, char *argv[])
{
    vicmil::set_app_init(init);
    vicmil::set_app_update(update);
    vicmil::app_start();
    return 0;
}