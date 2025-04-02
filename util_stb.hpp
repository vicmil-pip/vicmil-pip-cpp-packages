
// ============================================================
//                           Include
// ============================================================

#pragma once
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb/stb_truetype.h"

#include "util_std.hpp"
#include "util_std_bonus.hpp"

namespace vicmil
{
    // ============================================================
    //                           Loading images
    // ============================================================

    void _write_to_vector(void *vector_ptr, void *data, int size)
    {
        std::vector<unsigned char> *vec = (std::vector<unsigned char> *)vector_ptr;
        // PrintExpr(vicmil::to_binary_str(vec));
        vec->resize(size);
        memcpy(&(*vec)[0], data, size);
    }

    ImageRGBA_UChar ImageRGBA_UChar_load_png_from_file(std::string filename)
    {
        // If it failed to load the file, the width will be zero for the returned image object
        ImageRGBA_UChar return_image = ImageRGBA_UChar();
        int w = 0;
        int h = 0;
        int n = 0;
        int comp = 4; // r, g, b, a
        unsigned char *data = stbi_load(filename.c_str(), &w, &h, &n, comp);
        return_image.resize(w, h);
        if (w != 0)
        {
            return_image.set_pixel_data(data, w * h * 4);
        }
        stbi_image_free(data);
        return return_image;
    }

    void ImageRGBA_UChar_save_as_png(const ImageRGBA_UChar &image, std::string filename)
    {
        int comp = 4; // r, g, b, a
        const void *data = image.get_pixel_data_const();
        int stride_in_bytes = 0;
        stbi_write_png(filename.c_str(), image.w, image.h, comp, data, stride_in_bytes);
    }

    std::vector<unsigned char> ImageRGBA_UChar_to_png_as_bytes(const ImageRGBA_UChar &image)
    {
        int comp = 4; // r, g, b, a
        const void *data = &image.pixels[0];
        int stride_in_bytes = 0;
        stbi_write_func &write_func = _write_to_vector;

        std::vector<unsigned char> vec = std::vector<unsigned char>();
        std::vector<unsigned char> *vec_ptr = &vec;
        stbi_write_png_to_func(write_func, vec_ptr, image.w, image.h, comp, data, stride_in_bytes);
        return vec;
    }

    ImageRGBA_UChar ImageRGBA_UChar_png_as_bytes_to_image(const unsigned char *bytes, int length)
    {
        int w;
        int h;
        int n;
        int comp = 4; // r, g, b, a

        ImageRGBA_UChar return_image = ImageRGBA_UChar();

        unsigned char *data = stbi_load_from_memory(bytes, length, &w, &h, &n, comp);
        return_image.resize(w, h);
        return_image.set_pixel_data(data, w * h * 4);
        stbi_image_free(data);
        return return_image;
    }

    ImageRGBA_UChar ImageRGBA_UChar_png_as_bytes_to_image(const std::vector<unsigned char> &data)
    {
        return ImageRGBA_UChar_png_as_bytes_to_image(&data[0], data.size());
    }

    // ============================================================
    //                           Loading fonts
    // ============================================================

    // For loading true type fonts (.ttf)
    struct FontLoader
    {
        stbtt_fontinfo info;
        std::vector<unsigned char> font_data;

        // Calculated from line height
        int line_height;
        float scale;
        int ascent; // Line spacing in y direction
        int descent;
        int lineGap;

        void load_font_from_memory(unsigned char *fontBuffer_, int size, int line_height_ = 64)
        {
            // Load font into buffer
            font_data.resize(size);
            memcpy(&font_data[0], fontBuffer_, size);

            // Prepare font
            if (!stbtt_InitFont(&info, &font_data[0], 0))
            {
                printf("failed\n");
            }
            set_line_height(line_height_); // Set default line height
        }
        void load_font_from_file(std::string filepath, int line_height_ = 64)
        {
            vicmil::FileManager file = vicmil::FileManager(filepath);
            std::vector<char> data = file.read_entire_file();
            load_font_from_memory((unsigned char *)&data[0], data.size(), line_height_);
        }

        void set_line_height(int new_line_height)
        {
            line_height = new_line_height;
            scale = stbtt_ScaleForPixelHeight(&info, line_height);
            int ascent; // Line spacing in y direction
            int descent;
            int lineGap;
            stbtt_GetFontVMetrics(&info, &ascent, &descent, &lineGap);
            ascent = roundf(ascent * scale);
            descent = roundf(descent * scale);
        }

        void _get_character_advancement(const int character, int *advanceWidth, int *leftSideBearing)
        {
            // Advance width is how much to advance to the right
            // leftSideBearing means that it overlaps a little with the previous character
            stbtt_GetCodepointHMetrics(&info, character, advanceWidth, leftSideBearing);
            *advanceWidth = roundf(*advanceWidth * scale);
            *leftSideBearing = roundf(*leftSideBearing * scale);
        }

        int _get_kernal_advancement(const int character1, const int character2)
        {
            int kern = stbtt_GetCodepointKernAdvance(&info, character1, character2);
            return roundf(kern * scale);
        }

        RectT<int> _get_character_bounding_box(const int character)
        {
            int ax;
            int lsb;
            stbtt_GetCodepointHMetrics(&info, character, &ax, &lsb);

            // Get bounding box for character (may be offset to account for chars that dip above or below the line)
            int c_x1, c_y1, c_x2, c_y2;
            stbtt_GetCodepointBitmapBox(&info, character, scale, scale, &c_x1, &c_y1, &c_x2, &c_y2);
            return RectT<int>(c_x1, c_y1, c_x2 - c_x1, c_y2 - c_y1);
        }

        // Get image of character
        ImageRGBA_UChar get_character_image_rgba(const int character, ColorRGBA_UChar color_mask = ColorRGBA_UChar(255, 255, 255, 255))
        {
            RectT<int> bounding_box = _get_character_bounding_box(character);
            ImageRGBA_UChar return_image = ImageRGBA_UChar();
            std::vector<unsigned char> pixels_char_vec = std::vector<unsigned char>();
            pixels_char_vec.resize(bounding_box.w * bounding_box.h);
            stbtt_MakeCodepointBitmap(&info, (unsigned char *)&pixels_char_vec[0], bounding_box.w, bounding_box.h, bounding_box.w, scale, scale, character);
            return_image.resize(bounding_box.w, bounding_box.h);
            for (int i = 0; i < pixels_char_vec.size(); i++)
            {
                return_image.pixels[i] = ColorRGBA_UChar(color_mask.r, color_mask.g, color_mask.b, pixels_char_vec[i] * color_mask.a / 255.0);
            }
            return return_image;
        }

        // Get where font images in a text should be placed.
        // Some fonts may take into consideration which letters are next to each other, so-called font kerning
        // Characters are specified in unicode(but normal ascii will be treated as usual)
        std::vector<RectT<int>> get_character_image_positions(const std::vector<int> characters)
        {
            std::vector<RectT<int>> return_vec = {};
            return_vec.reserve(characters.size());

            int x = 0;
            for (int i = 0; i < characters.size(); i++)
            {
                // Get bounding box for character
                RectT<int> image_pos = _get_character_bounding_box(characters[i]);
                image_pos.x += x;
                image_pos.y += ascent;

                int advanceWidth;
                int leftSideBearing;
                _get_character_advancement(characters[i], &advanceWidth, &leftSideBearing);
                image_pos.x += leftSideBearing;

                // Push back bounding box
                return_vec.push_back(image_pos);

                // Increment position if there is another letter after
                if (i + 1 != characters.size())
                {
                    x += advanceWidth;
                    x += _get_kernal_advancement(characters[i], characters[i + 1]);
                }
            }
            return return_vec;
        }

        // Get the glyph index of character
        // (Can be used to determine if two letters correspond to the same font image)
        int get_glyph_index(const int character)
        {
            int glyphIndex = stbtt_FindGlyphIndex(&info, character);
            // if (glyphIndex == 0) {
            //     Print("Glyph not found for codepoint: " << character);
            // }
            return glyphIndex;
        }
        // Determine if a letter/character/unicode character is a part of the loaded font
        bool character_is_part_of_font(const int character)
        {
            return get_glyph_index(character) != 0;
        }
    };
}