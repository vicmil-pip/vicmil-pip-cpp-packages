#include "util_vicmil_gui.hpp"
#include "Noto_Sans_Mono_CJK_JP_Regular.otf.hpp"

vicmil::Window window;
vicmil::DefaultGpuPrograms gpu_programs;

std::shared_ptr<vicmil::MultiFontLoader> font_loader;
std::shared_ptr<vicmil::ImageManager> image_manager;
std::shared_ptr<vicmil::FontImageManager> font_image_manager;
vicmil::TextManager text_manager;

std::vector<vicmil::CoordTexCoord_XYZUV_f> tex_verticies;
std::vector<vicmil::CoordColor_XYZRGBA_f> color_verticies;

void update()
{
    std::vector<SDL_Event> events = vicmil::update_SDL();
    vicmil::clear_screen();

    image_manager.get()->update_gpu_image_with_cpu_image();
    gpu_programs.draw_2d_CoordTexCoord_XYZUV_f_vertex_buffer(tex_verticies, image_manager.get()->gpu_image);
    gpu_programs.draw_2d_CoordColor_XYZRGBA_f_vertex_buffer(color_verticies);

    window.show_on_screen();
}

void init()
{
    // Init SDL and Window
    vicmil::init_SDL();
    window = vicmil::Window(512, 512, "Hello rectangle minimal");

    gpu_programs = vicmil::DefaultGpuPrograms();
    gpu_programs.init_default_gpu_programs();

    // Load fonts
    font_loader = std::make_shared<vicmil::MultiFontLoader>();
    font_loader.get()->load_font_from_memory(get_NOTO_SANS_MONO_CJK_JP_REGULAR_OTF_data(), get_NOTO_SANS_MONO_CJK_JP_REGULAR_OTF_size());

    // Setup image managers
    image_manager = std::make_shared<vicmil::ImageManager>(512, 512);
    font_image_manager = std::make_shared<vicmil::FontImageManager>(image_manager, font_loader);

    // Load text
    std::string text = "abc\n日\n本\n語";
    std::vector<int> text_unicode = vicmil::utf8ToUnicodeCodePoints(text);

    text_manager = vicmil::TextManager(font_image_manager);
    text_manager.set_boundry(vicmil::GuiEngine::Rect(0, 0, 400, 400));
    text_manager.set_scroll_offset(10, 200);

    text_manager.update_text_positions(text_unicode);
    Print(text_manager._draw_positions[0].to_string());

    tex_verticies = text_manager.get_draw_vec(512, 512, 1);

    PrintExpr(text_manager.get_line_height());
    PrintExpr(text_manager.get_line_spacing());
    PrintExpr(text_manager._draw_positions[0].y);

    // Get cursor position
    vicmil::RectT<float> cursor_pos = text_manager.get_cursor_pos_gl(1, 512, 512);
    vicmil::add_color_rect_to_triangle_buffer(color_verticies, cursor_pos, 2, 128, 128, 128, 255);

    // Save the result
    std::vector<unsigned char> image_png = vicmil::ImageRGBA_UChar_to_png_as_bytes(image_manager.get()->cpu_image);
    vicmil::download_file("out_img.png", image_png);

    Print(font_image_manager.get()->get_unicode_image_pos_pixels(text_unicode[0]).to_string());

    vicmil::setup_fullscreen_canvas();
}

int main(int argc, char *argv[])
{
    vicmil::set_app_init(init);
    vicmil::set_app_update(update);
    vicmil::app_start();
    return 0;
}