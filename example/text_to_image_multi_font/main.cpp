#include <iostream>
#include "NotoSansMono-Regular.ttf.hpp"
#include "Noto Sans Mono CJK JP Regular.otf.hpp"
#include "util_stb.hpp"

int main()
{
    std::cout << "Loading fonts" << std::endl;
    vicmil::MultiFontLoader font_loader;
    // Try to load characters from the first font, if it fails, load it from the second font etc. etc.
    font_loader.load_font_from_memory(get_NOTOSANSMONO_REGULAR_TTF_data(), get_NOTOSANSMONO_REGULAR_TTF_size());
    font_loader.load_font_from_memory(get_NOTO_SANS_MONO_CJK_JP_REGULAR_OTF_data(), get_NOTO_SANS_MONO_CJK_JP_REGULAR_OTF_size());

    std::cout << "Loading text" << std::endl;
    std::string text = "abc日本語";
    std::vector<int> text_unicode = vicmil::utf8ToUnicodeCodePoints(text);
    std::vector<vicmil::RectT<int>> text_positions = font_loader.get_character_image_positions(text_unicode);

    std::cout << "Creating output image" << std::endl;
    int min_x = 0;
    int max_x = 0;
    int min_y = 0;
    int max_y = 0;
    for (int i = 0; i < text_positions.size(); i++)
    {
        min_x = std::min(min_x, text_positions[i].min_x());
        max_x = std::max(max_x, text_positions[i].max_x());
        min_y = std::min(min_y, text_positions[i].min_y());
        max_y = std::max(max_y, text_positions[i].max_y());
    }
    vicmil::ImageRGBA_UChar image_to_save;
    image_to_save.resize(max_x - min_x, max_y - min_y);

    std::cout << "w: " << image_to_save.w << std::endl;
    std::cout << "h: " << image_to_save.h << std::endl;

    std::cout << "Copy characters to output image" << std::endl;
    for (int i = 0; i < text_positions.size(); i++)
    {
        int x = text_positions[i].x - min_x;
        int y = text_positions[i].y - min_y;
        vicmil::ImageRGBA_UChar image = font_loader.get_character_image_rgba(text_unicode[i]);
        image.copy_to_image(&image_to_save, x, y);
    }

    std::cout << "Save output image" << std::endl;
    vicmil::ImageRGBA_UChar_save_as_png(image_to_save, "out_img.png");
    return 0;
}