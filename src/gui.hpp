// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "input.hpp"
#include "fonts.hpp"
#include "array.hpp"

class gui {
public:
    static constexpr u32 MAX_CONSOLE_LINE_LENGTH = 200;
    static constexpr u32 MAX_N_CONSOLE_LINES = 30;
    static constexpr char CONSOLE_SYMBOL[] = "> ";
    static constexpr size_t CONSOLE_SYMBOL_LENGTH = 2;

    static constexpr const char *MAIN_FONT_REGULAR = "SofiaProRegular.otf";
    static constexpr const char *MAIN_FONT_BOLD = "SofiaProBold.otf";
    static constexpr f32 LINE_SPACING_FACTOR = 1.8f;
    static constexpr f32 CONSOLE_LINE_SPACING_FACTOR = 1.2f;

    static constexpr u32 VERTEX_LENGTH = 8;

    static constexpr v2 TEXT_SHADOW_OFFSET = v2(1.0f);

    static constexpr v4 WINDOW_BG_COLOR = v4(0.20f, 0.20f, 0.20f, 1.00f);
    static constexpr v4 CONSOLE_BG_COLOR = v4(0.10f, 0.10f, 0.10f, 0.95f);
    static constexpr v4 MAIN_COLOR = v4(0.00f, 0.33f, 0.93f, 1.00f);
    static constexpr v4 MAIN_DARKEN_COLOR = v4(0.00f, 0.23f, 0.83f, 1.00f);
    static constexpr v4 MAIN_HOVER_COLOR = v4(0.00f, 0.43f, 1.00f, 1.00f);
    static constexpr v4 MAIN_ACTIVE_COLOR = v4(0.00f, 0.13f, 0.73f, 1.00f);
    static constexpr v4 LIGHT_TEXT_COLOR = v4(1.0f, 1.0f, 1.0f, 1.0f);
    static constexpr v4 LIGHT_COLOR = v4(0.9f, 0.9f, 0.9f, 1.0f);
    static constexpr v4 LIGHT_HOVER_COLOR = v4(1.0f, 1.0f, 1.0f, 1.0f);
    static constexpr v4 LIGHT_ACTIVE_COLOR = v4(0.7f, 0.7f, 0.7f, 1.0f);
    static constexpr v4 LIGHT_DARKEN_COLOR = v4(0.4f, 0.4f, 0.4f, 1.0f);

    static constexpr v2 BUTTON_AUTOSIZE_PADDING = v2(20.0f, 20.0f);
    static constexpr v2 BUTTON_DEFAULT_BORDER = v2(0.0f);

    static constexpr f32 NAMED_VALUE_NAME_WIDTH = 250.0f;
    static constexpr f32 TOGGLE_SPACING = 20.0f;
    static constexpr v2 TOGGLE_BUTTON_SIZE = v2(20.0f);
    static constexpr v2 TOGGLE_BUTTON_DEFAULT_BORDER = v2(0.0f);

    static constexpr f32 MAX_CONSOLE_LOG_HEIGHT = 350.0f;
    static constexpr v2 CONSOLE_PADDING = v2(10.0f);

    struct Container {
        const char *title;
        v2 position;
        // Dimensions include padding, title bar etc.
        v2 dimensions;
        v2 content_dimensions;
        v2 next_element_position;
        // The direction defines the main and orthogonal axes.
        // The main axis is the one elements are successively positioned on.
        // The main axis has a 1.0f, while the orthogonal axis has a 0.0f.
        v2 direction;
        v2 padding;
        f32 title_bar_height;
        u32 n_elements;
        f32 element_margin;
    };

    struct GameConsole {
        bool is_enabled;
        char log[gui::MAX_N_CONSOLE_LINES][gui::MAX_CONSOLE_LINE_LENGTH];
        u32 idx_log_start;
        u32 idx_log_end;
    };

    struct State {
        Array<fonts::FontAsset> *font_assets;
        iv2 texture_atlas_size;
        GameConsole console;

        input::CursorType requested_cursor;
        v2 window_dimensions;

        // Containers
        Array<Container> containers;
        Container *container_being_moved;

        // Heading
        f32 heading_opacity;
        const char *heading_text;
        f32 heading_fadeout_duration;
        f32 heading_fadeout_delay;
    };

    static void update_screen_dimensions(
        u32 new_window_width, u32 new_window_height
    );
    static void update_mouse_button();
    static void update_mouse();
    static void clear();
    static void update();
    static Container * make_container(const char *title, v2 position);
    static void draw_heading(const char *str, v4 color);
    static void tick_heading();
    static bool draw_toggle(
        Container *container,
        const char *text,
        bool toggle_state
    );
    static void draw_named_value(
        Container *container,
        const char *name_text,
        const char *value_text
    );
    static void draw_body_text(Container *container, const char *text);
    static bool draw_button(Container *container, const char *text);
    static void draw_console(char *console_input_text);
    static bool is_console_enabled();
    static void set_console_enabled(bool val);
    static void log(const char *format, ...);
    static void set_heading(
        const char *text, f32 opacity,
        f32 fadeout_duration, f32 fadeout_delay
    );
    static void init(
        memory::Pool *memory_pool,
        gui::State* gui_state,
        iv2 texture_atlas_size,
        Array<fonts::FontAsset> *font_assets,
        u32 window_width, u32 window_height
    );

private:
    static void request_cursor(input::CursorType cursor);
    static void set_cursor();
    static void push_vertices(f32 *vertices, u32 n_vertices);
    static v2 get_text_dimensions(fonts::FontAsset *font_asset, char const *str);
    static v2 center_bb(v2 container_position, v2 container_dimensions, v2 element_dimensions);
    static v2 add_element_to_container(Container *container, v2 element_dimensions);
    static void draw_rect(v2 position, v2 dimensions, v4 color);
    static void draw_text(
        char const *font_name, char const *str,
        v2 position,
        v4 color
    );
    static void draw_text_shadow(
        char const *font_name, char const *str,
        v2 position,
        v4 color
    );
    static void draw_container(Container *container);
    static void draw_line(v2 start, v2 end, f32 thickness, v4 color);
    static void draw_frame(v2 position, v2 bottomright, v2 thickness, v4 color);

    static gui::State *state;
};

