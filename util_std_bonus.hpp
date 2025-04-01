#pragma once

#include "util_std.hpp"

namespace vicmil
{
    // ============================================================
    //                       Images and Colors
    // ============================================================
    struct ColorRGBA_UChar
    {
        unsigned char r = 0;
        unsigned char g = 0;
        unsigned char b = 0;
        unsigned char a = 0;
        ColorRGBA_UChar() {}
        ColorRGBA_UChar(int r_, int g_, int b_, int a_)
        {
            r = r_;
            g = g_;
            b = b_;
            a = a_;
        }
        std::string to_string()
        {
            return vicmil::vec_to_str<int>({r, g, b});
        }
        bool operator==(const ColorRGBA_UChar &other) const
        {
            return r == other.r &&
                   g == other.g &&
                   b == other.b &&
                   a == other.a;
        }
    };

    struct ImageRGBA_UChar
    {
        int w;
        int h;
        std::vector<ColorRGBA_UChar> pixels;
        void resize(unsigned int new_width, unsigned int new_height)
        {
            // Note! Will not preserve content of image. Only the pixel vector will be resized
            w = new_width;
            h = new_height;
            pixels.resize(new_width * new_height);
        }
        void copy_to_image(ImageRGBA_UChar *other_image, int x, int y)
        {
            // Copy this image to other image, so that this image start corner is at x, y of other image
            int copy_max_x = std::min(other_image->w - x, w);
            int copy_max_y = std::min(other_image->h - y, h);
            for (int x2 = 0; x2 < copy_max_x; x2++)
            {
                for (int y2 = 0; y2 < copy_max_y; y2++)
                {
                    *other_image->get_pixel(x2 + x, y2 + y) = *get_pixel(x2, y2);
                }
            }
        }
        ColorRGBA_UChar *get_pixel(int x, int y)
        {
            return &pixels[y * w + x];
        }
        unsigned char *get_pixel_data()
        {
            return (unsigned char *)((void *)(&pixels[0]));
        }
        const unsigned char *get_pixel_data_const() const
        {
            return (const unsigned char *)((void *)(&pixels[0]));
        }
        void set_pixel_data(unsigned char *data, int byte_count)
        {
            assert(byte_count == pixels.size() * sizeof(ColorRGBA_UChar));
            std::memcpy(&pixels[0], data, byte_count);
        }
        void flip_vertical()
        {
            for (int x = 0; x < w; x++)
            {
                for (int y = 0; y < h / 2; y++)
                {
                    ColorRGBA_UChar tmp = *get_pixel(x, y);
                    *get_pixel(x, y) = *get_pixel(x, h - y - 1);
                    *get_pixel(x, h - y - 1) = tmp;
                }
            }
        }
        // Set the entire image to the selected color
        void fill(ColorRGBA_UChar new_color)
        {
            for (int i = 0; i < pixels.size(); i++)
            {
                pixels[i] = new_color;
            }
        }
    };

    // Image of floats, typically in the range [0,1], can be used to store depth images for example
    struct Image_float
    {
        int w;
        int h;
        std::vector<float> pixels;
        void resize(unsigned int new_width, unsigned int new_height)
        {
            pixels.resize(new_width * new_height);
        }
        float *get_pixel(int x, int y)
        {
            return &pixels[y * w + x];
        }
        float *get_pixel_data()
        {
            return &pixels[0];
        }
        const float *get_pixel_data_const() const
        {
            return &pixels[0];
        }
        void set_pixel_data(float *data, int byte_count)
        {
            assert(byte_count == pixels.size() * sizeof(float));
            std::memcpy(&pixels[0], data, byte_count);
        }
        void flip_vertical()
        {
            for (int x = 0; x < w; x++)
            {
                for (int y = 0; y < h / 2; y++)
                {
                    float tmp = *get_pixel(x, y);
                    *get_pixel(x, y) = *get_pixel(x, h - y - 1);
                    *get_pixel(x, h - y - 1) = tmp;
                }
            }
        }
        ImageRGBA_UChar to_image_rgba_uchar()
        {
            ImageRGBA_UChar new_image;
            // Assume float values are in the range 0 and 1
            // make interval between green and red, where red is 0 and green is 1
            new_image.resize(w, h);
            for (int i = 0; i < pixels.size(); i++)
            {
                float pixel_value = std::max(std::min(pixels[i], 1.0f), 0.0f); // Clip it in range 0 and 1
                new_image.pixels[i] = ColorRGBA_UChar((1.0 - pixel_value) * 255, pixel_value * 255, 0, 1);
            }
            return new_image;
        }
    };

    // ============================================================
    //              Common vertex buffer storage types
    // ============================================================
    struct XYZ_f
    {
        float x = -100;
        float y = -100;
        float z = -100;
        XYZ_f() {}
        XYZ_f(float x_, float y_, float z_)
        {
            x = x_;
            y = y_;
            z = z_;
        }
    };

    struct Coord_XYZ_f : public XYZ_f
    {
    };
    struct Normal_XYZ_f : public XYZ_f
    {
    };

    struct TexCoord_UV_f
    {
        float u = -100;
        float v = -100;
        TexCoord_UV_f() {}
        TexCoord_UV_f(float u_, float v_)
        {
            u = u_;
            v = v_;
        }
    };

    struct ColorRGBA_f
    {
        float r = 0.0;
        float g = 1.0;
        float b = 0.0;
        float a = 1.0;
        ColorRGBA_f() {}
        ColorRGBA_f(float r_, float g_, float b_, float a_ = 1.0)
        {
            r = r_;
            g = g_;
            b = b_;
            a = a_;
        }
    };

    struct CoordColor_XYZRGBA_f
    {
        float x = 0.0;
        float y = 0.0;
        float z = 0.0;
        float r = 0.0;
        float g = 1.0;
        float b = 0.0;
        float a = 1.0;
        CoordColor_XYZRGBA_f() {}
        CoordColor_XYZRGBA_f(float x_, float y_, float z_, float r_, float g_, float b_, float a_ = 1.0)
        {
            x = x_;
            y = y_;
            z = z_;
            r = r_;
            g = g_;
            b = b_;
            a = a_;
        }
    };

    struct CoordTexCoord_XYZUV_f
    {
        float x = 0.0;
        float y = 0.0;
        float z = 0.0;
        float u = 0.0;
        float v = 1.0;
        CoordTexCoord_XYZUV_f() {}
        CoordTexCoord_XYZUV_f(float x_, float y_, float z_, float u_, float v_)
        {
            x = x_;
            y = y_;
            z = z_;
            u = u_;
            v = v_;
        }
    };
}
