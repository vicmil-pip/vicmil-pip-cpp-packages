#pragma once

#include "util_stb.hpp"
#include "util_bin_packing.hpp"
#include "util_miniz.hpp"
#include "util_opengl.hpp"
#include "util_socketio.hpp"
#include "util_obj_loader.hpp"

namespace vicmil
{
    class ImageManager
    {
    public:
        vicmil::ImageRGBA_UChar cpu_image;
        vicmil::GPUImage gpu_image;
        vicmil::RectPacker _rect_packer;
        std::map<std::string, vicmil::ImageRGBA_UChar> _images_raw;
        bool cpu_image_updated = true;
        ImageManager() {}
        ImageManager(int w, int h)
        {
            if (!(vicmil::is_power_of_two(w) && vicmil::is_power_of_two(h)))
            {
                ThrowError("Image size must be power of 2: " << w << ", " << h);
            }
            cpu_image.resize(w, h);
            _rect_packer = vicmil::RectPacker(w, h);
        }

        bool add_image(std::string image_name, vicmil::ImageRGBA_UChar image)
        {
            if (contains_image(image_name))
            {
                return false; // Image already exists
            }
            if (!_rect_packer.add_rect(image_name, image.w, image.h))
            {
                return false; // There is not enough space for the image on the larger canvas
            }
            _images_raw[image_name] = image;
            vicmil::RectT<int> cpu_image_pos = _rect_packer.get_rect(image_name);
            image.copy_to_image(&cpu_image, cpu_image_pos.x, cpu_image_pos.y);
            cpu_image_updated = true;
            return true;
        }

        void remove_image(std::string image_name)
        {
            if (!contains_image(image_name))
            {
                return; // There is no such image
            }
            _images_raw.erase(image_name);
            _rect_packer.remove_rect(image_name);
        }

        inline bool contains_image(std::string image_name)
        {
            return _images_raw.count(image_name) != 0;
        }

        inline vicmil::RectT<int> get_image_pos_pixels(std::string image_name)
        {
            if (!contains_image(image_name))
            {
                return vicmil::RectT<int>(0, 0, 0, 0); // There is no such image
            }
            return _rect_packer.get_rect(image_name);
        }

        inline vicmil::GuiEngine::RectGL get_image_pos_gl(std::string image_name)
        {
            vicmil::RectT<int> image_pos_p = get_image_pos_pixels(image_name);
            if (image_pos_p.w == 0)
            {
                return vicmil::GuiEngine::RectGL(0, 0, 0, 0); // Invalid image
            }
            return vicmil::GuiEngine::RectGL(
                image_pos_p.x / (double)cpu_image.w,
                image_pos_p.y / (double)cpu_image.h,
                image_pos_p.w / (double)cpu_image.w,
                image_pos_p.h / (double)cpu_image.h);
        }

        void update_gpu_image_with_cpu_image()
        {
            if (!cpu_image_updated)
            {
                return; // No need to push a gpu image if the cpu image has not been updated
            }
            cpu_image_updated = false;
            if (gpu_image.texture.no_texture)
            {
                gpu_image = vicmil::GPUImage::from_CPUImage(cpu_image);
            }
            else
            {
                gpu_image.overwrite_with_CPUImage(cpu_image);
            }
        }
    };

    class FontImageManager
    {
    public:
        std::shared_ptr<vicmil::ImageManager> image_manager;
        std::shared_ptr<vicmil::MultiFontLoader> font_loader;

        FontImageManager() {}
        FontImageManager(
            std::shared_ptr<vicmil::ImageManager> image_manager_,
            std::shared_ptr<vicmil::MultiFontLoader> font_loader_) : image_manager(image_manager_), font_loader(font_loader_) {}

        inline std::string _get_image_name(int unicode)
        {
            return "U_" + std::to_string(unicode);
        }

        /*
        Adds the font image to the image manager if it does not already exist
        Then get the image position
        NOTE! You need to call image_manager.update_gpu_image_with_cpu_image() for it to load to the gpu
            (Not ideal to do for every character, since the operation is expensive)
        */
        inline void _make_sure_image_exists(int unicode)
        {
            std::string image_name = _get_image_name(unicode);
            if (!image_manager.get()->contains_image(image_name))
            {
                vicmil::ImageRGBA_UChar image = font_loader.get()->get_character_image_rgba(unicode);
                image_manager.get()->add_image(image_name, image);
            }
        }

        inline vicmil::GuiEngine::RectGL get_unicode_image_pos_gl(int unicode)
        {
            _make_sure_image_exists(unicode);
            std::string image_name = _get_image_name(unicode);
            return image_manager.get()->get_image_pos_gl(image_name);
        }

        inline vicmil::RectT<int> get_unicode_image_pos_pixels(int unicode)
        {
            _make_sure_image_exists(unicode);
            std::string image_name = _get_image_name(unicode);
            return image_manager.get()->get_image_pos_pixels(image_name);
        }
    };

    struct WidgetData
    {
        // graphics
        std::vector<vicmil::CoordColor_XYZRGBA_f> color_triangles = {};
        std::vector<vicmil::CoordTexCoord_XYZUV_f> texture_triangles = {};

        // User input
        std::string gui_element_at_mouse;
        MouseState mouse_state;
        KeyboardState keyboard_state;
        std::vector<SDL_Event> events;
        bool mouse_left_clicked = false;
        bool mouse_right_clicked = false;

        // Other
        std::shared_ptr<vicmil::ImageManager> image_manager;
        std::shared_ptr<FontImageManager> font_image_manager;
        std::shared_ptr<vicmil::GuiEngine> gui_engine;
        std::shared_ptr<vicmil::MultiFontLoader> font_loader_;
        vicmil::RandomNumberGenerator rand_gen = vicmil::RandomNumberGenerator();
        WidgetData()
        {
            image_manager = std::make_shared<vicmil::ImageManager>(2048, 2048);
            font_loader_ = std::make_shared<vicmil::MultiFontLoader>();
            font_image_manager = std::make_shared<FontImageManager>(image_manager, font_loader_);
            gui_engine = std::make_shared<vicmil::GuiEngine>();
        }
    };
    class Widget
    {
    public:
        virtual void update() = 0; // Update the widget and add new draw triangles
        virtual std::string get_name() = 0;
        static std::string get_unique_id()
        {
            static int counter = 0;
            counter++;
            return std::to_string(counter);
        }
    };
    class WidgetManager
    {
    public:
        std::shared_ptr<WidgetData> data = std::make_shared<WidgetData>();
        std::map<std::string, std::weak_ptr<Widget>> widgets;
        WidgetManager() {}
        void update(vicmil::Window &window, std::vector<SDL_Event> &events)
        {
            // Reset draw triangles
            data.get()->texture_triangles = {};
            data.get()->color_triangles = {};

            // Handle user input
            data.get()->mouse_state = MouseState(window.window);
            data.get()->keyboard_state = KeyboardState();
            data.get()->events = events;
            data.get()->mouse_left_clicked = vicmil::mouse_left_clicked(events);
            data.get()->mouse_right_clicked = vicmil::mouse_right_clicked(events);

            // Update gui engine
            int window_w, window_h;
            vicmil::get_window_size(window.window, window_w, window_h);
            data.get()->gui_engine.get()->set_screen_size(window_w, window_h);
            data.get()->gui_element_at_mouse = data.get()->gui_engine.get()->get_xy_element(
                data.get()->mouse_state.x(),
                data.get()->mouse_state.y());
            data.get()->gui_engine.get()->build();

            // Update all the widgets
            std::vector<std::string> widgets_to_remove;
            for (auto widget : widgets)
            {
                std::weak_ptr<Widget> widget_ptr = widget.second;
                if (widget_ptr.expired())
                {
                    widgets_to_remove.push_back(widget.first);
                    continue;
                }
                widget_ptr.lock().get()->update();
            }

            // Remove widgets which have expired
            for (int i = 0; i < widgets_to_remove.size(); i++)
            {
                widgets.erase(widgets_to_remove[i]);
            }
        }
        void draw(vicmil::DefaultGpuPrograms &gpu_programs)
        {
            if (data.get()->color_triangles.size())
            {
                gpu_programs.draw_2d_CoordColor_XYZRGBA_f_vertex_buffer(data.get()->color_triangles);
            }
            if (data.get()->texture_triangles.size())
            {
                gpu_programs.draw_2d_CoordTexCoord_XYZUV_f_vertex_buffer(data.get()->texture_triangles, data.get()->image_manager.get()->gpu_image);
            }
        }
        void add_widget(std::weak_ptr<Widget> widget)
        {
            widgets[widget.lock().get()->get_name()] = widget;
        }
    };
    class ButtonWidget : public Widget
    {
    public:
        int _w = 100;
        int _h = 100;
        std::string _attach_to = "screen";
        vicmil::GuiEngine::attach_location _attach_location = vicmil::GuiEngine::attach_location::o_TopLeft_e_TopLeft;
        int _layer = 1;
        bool _updated = true;
        std::shared_ptr<WidgetData> data;
        std::string widget_name = "undefined";
        std::string get_name() override { return widget_name; }
        ColorRGBA_UChar color = ColorRGBA_UChar(255, 0, 0, 255);

        ButtonWidget() {}
        ButtonWidget(std::shared_ptr<WidgetData> data_)
        {
            data = data_;
            widget_name = "button_widget" + get_unique_id();
        }
        virtual void pressed()
        {
            Print("Button was pressed");
        }

        void attach_to(std::string attach_to_, vicmil::GuiEngine::attach_location attach_location_)
        {
            _attach_to = attach_to_;
            _attach_location = attach_location_;
            _updated = true;
        }

        void set_size(int w, int h)
        {
            _w = w;
            _h = h;
            _updated = true;
        }

        void set_layer(int layer)
        {
            _layer = layer;
            _updated = true;
        }

        void update() override
        {
            // See if the user has pressed left click, and have mouse over button
            if (data.get()->gui_element_at_mouse == widget_name && data.get()->mouse_left_clicked)
            {
                pressed();
            }

            if (_updated)
            {
                data.get()->gui_engine.get()->element_attach(widget_name, _w, _h, _attach_to, _attach_location, _layer);
                _updated = false;
            }

            draw();
        }
        virtual void draw()
        {
            // Add the graphics
            vicmil::add_color_rect_to_triangle_buffer(
                data.get()->color_triangles,                                   // Triangle buffer
                data.get()->gui_engine.get()->get_element_gl_pos(widget_name), // Position on screen
                _layer,                                                        // Layer on screen
                color.r, color.g, color.b, color.a);                           // Color
        }
    };
    class SwitchWidget : public ButtonWidget
    {
    public:
        bool _enabled = false;
        SwitchWidget() {}
        SwitchWidget(std::shared_ptr<WidgetData> data_)
        {
            data = data_;
            widget_name = "switch_widget" + get_unique_id();
        }
        void set_enabled(int enabled_)
        {
            _enabled = enabled_;
            if (_enabled)
            {
                color = ColorRGBA_UChar(0, 255, 0, 255);
            }
            else
            {
                color = ColorRGBA_UChar(255, 0, 0, 255);
            }
        }
        void pressed() override
        {
            set_enabled(!_enabled);
        }
    };
}