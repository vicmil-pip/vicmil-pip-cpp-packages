#include <iostream>
#include "NotoSansMono-Regular.ttf.hpp"
#include "util_stb.hpp"

int main()
{
    vicmil::FontLoader font_loader;
    font_loader.load_font_from_memory(get_NOTOSANSMONO_REGULAR_TTF_data(), get_NOTOSANSMONO_REGULAR_TTF_size());
    vicmil::ImageRGBA_UChar image = font_loader.get_character_image_rgba('a');
    vicmil::ImageRGBA_UChar_save_as_png(image, "a.png");
    std::cout << "hello world!" << std::endl;
    return 0;
}