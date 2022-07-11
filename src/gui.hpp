// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "input.hpp"
#include "fonts.hpp"
#include "array.hpp"

class gui {
public:
    static constexpr uint32 MAX_CONSOLE_LINE_LENGTH = 200;
    static constexpr uint32 MAX_N_CONSOLE_LINES = 30;
    static constexpr char CONSOLE_SYMBOL[] = "> ";
    static constexpr size_t CONSOLE_SYMBOL_LENGTH = 2;

    static constexpr const char *MAIN_FONT_REGULAR = "SofiaProRegular.otf";
    static constexpr const char *MAIN_FONT_BOLD = "SofiaProBold.otf";
    static constexpr real32 LINE_SPACING_FACTOR = 1.8f;
    static constexpr real32 CONSOLE_LINE_SPACING_FACTOR = 1.2f;

    static constexpr uint32 VERTEX_LENGTH = 8;

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

    static constexpr real32 NAMED_VALUE_NAME_WIDTH = 250.0f;
    static constexpr real32 TOGGLE_SPACING = 20.0f;
    static constexpr v2 TOGGLE_BUTTON_SIZE = v2(20.0f);
    static constexpr v2 TOGGLE_BUTTON_DEFAULT_BORDER = v2(0.0f);

    static constexpr real32 MAX_CONSOLE_LOG_HEIGHT = 350.0f;
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
        real32 title_bar_height;
        uint32 n_elements;
        real32 element_margin;
    };

    struct GameConsole {
        bool32 is_enabled;
        char log[gui::MAX_N_CONSOLE_LINES][gui::MAX_CONSOLE_LINE_LENGTH];
        uint32 idx_log_start;
        uint32 idx_log_end;
    };

    struct State {
        // NOTE: We're holding a pointer to another state here. What does this mean?
        InputState *input_state;
        Array<fonts::FontAsset> *font_assets;
        iv2 texture_atlas_size;

        GLFWcursor *requested_cursor;
        v2 window_dimensions;

        // Containers
        Array<Container> containers;
        Container *container_being_moved;

        // Heading
        real32 heading_opacity;
        const char *heading_text;
        real32 heading_fadeout_duration;
        real32 heading_fadeout_delay;
    };

    static void update_screen_dimensions(
        gui::State *gui_state,
        uint32 new_window_width, uint32 new_window_height
    );
    static void update_mouse_button(gui::State *gui_state);
    static void update_mouse(gui::State *gui_state);
    static void clear(gui::State *gui_state);
    static void update(gui::State *gui_state);
    static Container * make_container(
        gui::State *gui_state, const char *title, v2 position
    );
    static void draw_heading(
        gui::State *gui_state,
        const char *str,
        v4 color
    );
    static bool32 draw_toggle(
        gui::State *gui_state,
        Container *container,
        const char *text,
        bool32 *toggle_state
    );
    static void draw_named_value(
        gui::State *gui_state,
        Container *container,
        const char *name_text,
        const char *value_text
    );
    static void draw_body_text(
        gui::State *gui_state,
        Container *container,
        const char *text
    );
    static bool32 draw_button(
        gui::State *gui_state,
        Container *container,
        const char *text
    );
    static void draw_console(
        gui::State *gui_state,
        char *console_input_text
    );
    static void log(const char *format, ...);
    static void set_heading(
        gui::State *gui_state,
        const char *text, real32 opacity,
        real32 fadeout_duration, real32 fadeout_delay
    );
    static void init(
        MemoryPool *memory_pool,
        gui::State* gui_state,
        InputState *input_state,
        iv2 texture_atlas_size,
        Array<fonts::FontAsset> *font_assets,
        uint32 window_width, uint32 window_height
    );

private:
    static void request_cursor(gui::State *gui_state, GLFWcursor *cursor);
    static void set_cursor(gui::State *gui_state);
    static void push_vertices(gui::State *gui_state, real32 *vertices, uint32 n_vertices);
    static v2 get_text_dimensions(fonts::FontAsset *font_asset, char const *str);
    static v2 center_bb(v2 container_position, v2 container_dimensions, v2 element_dimensions);
    static v2 add_element_to_container(Container *container, v2 element_dimensions);
    static void draw_rect(gui::State *gui_state, v2 position, v2 dimensions, v4 color);
    static void draw_text(
        gui::State *gui_state,
        char const *font_name, char const *str,
        v2 position,
        v4 color
    );
    static void draw_text_shadow(
        gui::State *gui_state,
        char const *font_name, char const *str,
        v2 position,
        v4 color
    );
    static void draw_container(gui::State *gui_state, Container *container);
    static void draw_line(
        gui::State *gui_state,
        v2 start, v2 end,
        real32 thickness, v4 color
    );
    static void draw_frame(
        gui::State *gui_state,
        v2 position, v2 bottomright,
        v2 thickness, v4 color
    );

public:
    static gui::GameConsole con;
};

