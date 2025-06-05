#include "util_vicmil_gui.hpp"
#include "Noto Sans Mono CJK JP Regular.otf.hpp"

std::shared_ptr<vicmil::MultiFontLoader> font_loader;
std::shared_ptr<vicmil::ImageManager> image_manager;
std::shared_ptr<vicmil::FontImageManager> font_image_manager;

void update()
{
}

void init()
{
    // Load fonts
    font_loader = std::make_shared<vicmil::MultiFontLoader>();
    font_loader.get()->load_font_from_memory(get_NOTO_SANS_MONO_CJK_JP_REGULAR_OTF_data(), get_NOTO_SANS_MONO_CJK_JP_REGULAR_OTF_size());

    // Setup image managers
    image_manager = std::make_shared<vicmil::ImageManager>(512, 512);
    font_image_manager = std::make_shared<vicmil::FontImageManager>(image_manager, font_loader);

    // Load text
    std::string text = "abc日本語";
    std::vector<int> text_unicode = vicmil::utf8ToUnicodeCodePoints(text);

    // Load letters on one texture
    for (int i = 0; i < text_unicode.size(); i++)
    {
        font_image_manager.get()->get_unicode_image_pos_pixels(text_unicode[i]);
    }

    // Save the result
    std::vector<unsigned char> image_png = vicmil::ImageRGBA_UChar_to_png_as_bytes(image_manager.get()->cpu_image);
    vicmil::download_file("out_img.png", image_png);
}

int main(int argc, char *argv[])
{
    vicmil::set_app_init(init);
    vicmil::set_app_update(update);
    vicmil::app_start();
    return 0;
}