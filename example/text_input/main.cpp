#include "util_std.hpp"
#include "util_opengl.hpp"

vicmil::Window window;
vicmil::SdlTextInput text_input;

void update()
{
    std::vector<SDL_Event> events = vicmil::update_SDL();
    if (text_input.update_text_input(events))
    {
        Print("Update!");
        Print("\n"
              << text_input.get_text_utf8_with_cursor() << text_input.get_compose_text_utf8());
    }
}

void init()
{
    vicmil::init_SDL();
    window = vicmil::Window(512, 512, "Hello rectangle minimal");
    text_input.set_input_text_activated(true);

    // (Where the cursor is rendered)
    text_input.set_cursor_render_pos(vicmil::RectT<int>(100, 100, 1, 10));
}

int main(int argc, char *argv[])
{
    vicmil::set_app_init(init);
    vicmil::set_app_update(update);
    vicmil::app_start();
    return 0;
}