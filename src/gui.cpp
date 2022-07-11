// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "../src_external/glad/glad.h"
#include <GLFW/glfw3.h>
#include "gui.hpp"
#include "util.hpp"
#include "logs.hpp"
#include "intrinsics.hpp"


gui::State *gui::state = nullptr;


void
gui::update_screen_dimensions(uint32 new_window_width, uint32 new_window_height)
{
    gui::state->window_dimensions = v2(new_window_width, new_window_height);
}


void
gui::update_mouse_button()
{
    if (input::is_mouse_button_now_up(gui::state->input_state, GLFW_MOUSE_BUTTON_LEFT)) {
        gui::state->container_being_moved = nullptr;
    }
}


void
gui::update_mouse()
{
    if (gui::state->container_being_moved) {
        gui::state->container_being_moved->position += gui::state->input_state->mouse_offset;
    }
}


void
gui::update()
{
    set_cursor();
}


gui::Container *
gui::make_container(const char *title, v2 position)
{
    Container *container = nullptr;
    each (container_candidate, gui::state->containers) {
        if (strcmp(container_candidate->title, title) == 0) {
            container = container_candidate;
            break;
        }
    }

    if (container) {
        // Check if we need to set this container as being moved.
        if (
            input::is_mouse_in_bb(gui::state->input_state,
                container->position,
                container->position + v2(container->dimensions.x, container->title_bar_height)) &&
            input::is_mouse_button_now_down(gui::state->input_state, GLFW_MOUSE_BUTTON_LEFT)
        ) {
            gui::state->container_being_moved = container;
        }

        // Draw the container with the information from the previous frame
        // if there is anything in it.
        if (container->content_dimensions != v2(0.0f, 0.0f)) {
            draw_container(container);
        }
    } else {
        container = gui::state->containers.push();
        container->title = title;
        container->position = position;
        container->direction = v2(0.0f, 1.0f);
        container->padding = v2(20.0f);
        container->title_bar_height = 40.0f;
        container->element_margin = 20.0f;
    }

    // In all cases, clear this container.
    container->n_elements = 0;
    container->dimensions = container->padding * 2.0f;
    container->content_dimensions = v2(0.0f, 0.0f);
    container->next_element_position = container->position +
        container->padding +
        v2(0.0f, container->title_bar_height);

    return container;
}


void
gui::draw_heading(const char *str, v4 color)
{
    auto bb = center_bb(v2(0.0f, 0.0f),
        gui::state->window_dimensions,
        get_text_dimensions(fonts::get_by_name(gui::state->font_assets, "heading"), str));
    v2 position = v2(bb.x, 90.0f);
    draw_text_shadow("heading", str, position, color);
    draw_text("heading", str, position, color);
}


void
gui::tick_heading()
{
    if (gui::state->heading_opacity > 0.0f) {
        draw_heading(gui::state->heading_text,
            v4(0.0f, 0.33f, 0.93f, gui::state->heading_opacity));
        if (gui::state->heading_fadeout_delay > 0.0f) {
            gui::state->heading_fadeout_delay -= (real32)(*engine::g_dt);
        } else {
            gui::state->heading_opacity -=
                gui::state->heading_fadeout_duration * (real32)(*engine::g_dt);
        }
    }
}


bool32
gui::draw_toggle(Container *container, const char *text, bool32 toggle_state)
{
    bool32 is_pressed = false;

    v2 text_dimensions = get_text_dimensions(fonts::get_by_name(gui::state->font_assets, "body"), text);
    v2 button_dimensions = TOGGLE_BUTTON_SIZE + BUTTON_DEFAULT_BORDER * 2.0f;
    v2 dimensions = v2(button_dimensions.x + TOGGLE_SPACING + text_dimensions.x,
        max(button_dimensions.y, text_dimensions.y));

    v2 position = add_element_to_container(container, dimensions);

    v2 button_bottomright = position + button_dimensions;
    v2 text_centered_position = center_bb(position, button_dimensions, text_dimensions);
    v2 text_position = v2(position.x + button_dimensions.x + TOGGLE_SPACING,
        text_centered_position.y);

    v4 button_color;
    if (toggle_state) {
        button_color = MAIN_COLOR;
    } else {
        button_color = LIGHT_COLOR;
    }

    if (input::is_mouse_in_bb(gui::state->input_state, position, button_bottomright)) {
        request_cursor(gui::state->input_state->hand_cursor);
        if (toggle_state) {
            button_color = MAIN_HOVER_COLOR;
        } else {
            button_color = LIGHT_HOVER_COLOR;
        }

        if (input::is_mouse_button_now_down(gui::state->input_state, GLFW_MOUSE_BUTTON_LEFT)) {
            is_pressed = true;
        }

        if (input::is_mouse_button_down(gui::state->input_state, GLFW_MOUSE_BUTTON_LEFT)) {
            if (toggle_state) {
                button_color = MAIN_ACTIVE_COLOR;
            } else {
                button_color = LIGHT_ACTIVE_COLOR;
            }
        }
    }

    draw_frame(position, button_bottomright, TOGGLE_BUTTON_DEFAULT_BORDER, LIGHT_DARKEN_COLOR);
    draw_rect(position + TOGGLE_BUTTON_DEFAULT_BORDER,
        button_dimensions - (TOGGLE_BUTTON_DEFAULT_BORDER * 2.0f), button_color);
    draw_text("body", text, text_position, LIGHT_TEXT_COLOR);

    return is_pressed;
}


void
gui::draw_named_value(Container *container, const char *name_text, const char *value_text)
{
    v2 name_text_dimensions = get_text_dimensions(fonts::get_by_name(gui::state->font_assets, "body-bold"),
        name_text);
    v2 value_text_dimensions = get_text_dimensions(fonts::get_by_name(gui::state->font_assets, "body"),
        value_text);
    // Sometimes we draw a value which is a rapidly changing number.
    // We don't want to container to wobble in size back and forth, so we round
    // the size of the value text to the next multiple of 50.
    value_text_dimensions.x = util::round_to_nearest_multiple(value_text_dimensions.x, 50.0f);
    v2 dimensions = v2(value_text_dimensions.x + NAMED_VALUE_NAME_WIDTH,
        max(name_text_dimensions.y, value_text_dimensions.y));

    v2 position = add_element_to_container(container, dimensions);

    draw_text("body-bold", name_text, position, LIGHT_TEXT_COLOR);

    v2 value_text_position = position + v2(NAMED_VALUE_NAME_WIDTH, 0.0f);
    draw_text("body", value_text, value_text_position, LIGHT_TEXT_COLOR);
}


void
gui::draw_body_text(Container *container, const char *text)
{
    v2 dimensions = get_text_dimensions(fonts::get_by_name(gui::state->font_assets, "body"),
        text);
    v2 position = add_element_to_container(container, dimensions);
    draw_text("body", text, position, LIGHT_TEXT_COLOR);
}


bool32
gui::draw_button(Container *container, const char *text)
{
    bool32 is_pressed = false;

    v2 text_dimensions = get_text_dimensions(fonts::get_by_name(gui::state->font_assets, "body"), text);
    v2 button_dimensions = text_dimensions + BUTTON_AUTOSIZE_PADDING + BUTTON_DEFAULT_BORDER * 2.0f;

    v2 position = add_element_to_container(container, button_dimensions);

    v2 bottomright = position + button_dimensions;
    v2 text_position = center_bb(position, button_dimensions, text_dimensions);

    v4 button_color = MAIN_COLOR;

    if (input::is_mouse_in_bb(gui::state->input_state, position, bottomright)) {
        request_cursor(gui::state->input_state->hand_cursor);
        button_color = MAIN_HOVER_COLOR;

        if (input::is_mouse_button_now_down(gui::state->input_state, GLFW_MOUSE_BUTTON_LEFT)) {
            is_pressed = true;
        }

        if (input::is_mouse_button_down(gui::state->input_state, GLFW_MOUSE_BUTTON_LEFT)) {
            button_color = MAIN_ACTIVE_COLOR;
        }
    }

    draw_frame(position, bottomright, BUTTON_DEFAULT_BORDER, MAIN_DARKEN_COLOR);
    draw_rect(position + BUTTON_DEFAULT_BORDER,
        button_dimensions - (BUTTON_DEFAULT_BORDER * 2.0f), button_color);
    draw_text("body", text, text_position, LIGHT_TEXT_COLOR);

    return is_pressed;
}


void
gui::draw_console(char *console_input_text)
{
    if (!gui::state->console.is_enabled) {
        return;
    }

    fonts::FontAsset *font_asset = fonts::get_by_name(gui::state->font_assets, "body");
    real32 line_height = fonts::font_unit_to_px(font_asset->height);
    real32 line_spacing = floor(line_height * CONSOLE_LINE_SPACING_FACTOR);

    // Draw console log
    {
        v2 next_element_position = v2(CONSOLE_PADDING.x, MAX_CONSOLE_LOG_HEIGHT);

        draw_rect(v2(0.0f, 0.0f),
            v2(gui::state->window_dimensions.x, MAX_CONSOLE_LOG_HEIGHT),
            CONSOLE_BG_COLOR);

        uint32 idx_line = gui::state->console.idx_log_start;
        while (idx_line != gui::state->console.idx_log_end) {
            v2 text_dimensions = get_text_dimensions(font_asset, gui::state->console.log[idx_line]);
            next_element_position.y -= text_dimensions.y + line_spacing;
            draw_text("body", gui::state->console.log[idx_line], next_element_position, LIGHT_TEXT_COLOR);

            idx_line++;
            if (idx_line == MAX_N_CONSOLE_LINES) {
                idx_line = 0;
            }
        }
    }

    // Draw console input
    {
        real32 console_input_height = line_height + (2.0f * CONSOLE_PADDING.y);
        v2 console_input_position = v2(0.0f, MAX_CONSOLE_LOG_HEIGHT);

        draw_rect(console_input_position,
            v2(gui::state->window_dimensions.x, console_input_height),
            MAIN_DARKEN_COLOR);

        draw_text("body", console_input_text,
            console_input_position + CONSOLE_PADDING, LIGHT_TEXT_COLOR);
    }
}


bool
gui::is_console_enabled()
{
    return gui::state->console.is_enabled;
}


void
gui::set_console_enabled(bool val)
{
    gui::state->console.is_enabled = val;
}


void
gui::log(const char *format, ...)
{
    char text[MAX_CONSOLE_LINE_LENGTH];
    va_list vargs;
    va_start(vargs, format);
    vsnprintf(text, sizeof(text), format, vargs);
    va_end(vargs);

    // Fill array in back-to-front.
    if (gui::state->console.idx_log_start == 0) {
        gui::state->console.idx_log_start = MAX_N_CONSOLE_LINES - 1;
    } else {
        gui::state->console.idx_log_start--;
    }
    if (gui::state->console.idx_log_start == gui::state->console.idx_log_end) {
        if (gui::state->console.idx_log_end == 0) {
            gui::state->console.idx_log_end = MAX_N_CONSOLE_LINES - 1;
        } else {
            gui::state->console.idx_log_end--;
        }
    }
    strcpy(gui::state->console.log[gui::state->console.idx_log_start], text);
}


void
gui::set_heading(
    const char *text, real32 opacity,
    real32 fadeout_duration, real32 fadeout_delay
) {
    gui::state->heading_text = text;
    gui::state->heading_opacity = opacity;
    gui::state->heading_fadeout_duration = fadeout_duration;
    gui::state->heading_fadeout_delay = fadeout_delay;
}


void
gui::init(
    MemoryPool *memory_pool,
    gui::State* gui_state,
    InputState *input_state,
    iv2 texture_atlas_size,
    Array<fonts::FontAsset> *font_assets,
    uint32 window_width, uint32 window_height
) {
    gui::state = gui_state;

    gui::state->containers = Array<Container>(memory_pool, 32, "gui_containers");
    gui::state->input_state = input_state;
    gui::state->texture_atlas_size = texture_atlas_size;
    gui::state->font_assets = font_assets;
    gui::state->window_dimensions = v2(window_width, window_height);
    log("Hello world!");
}


void
gui::request_cursor(GLFWcursor *cursor)
{
    gui::state->requested_cursor = cursor;
}


void
gui::set_cursor()
{
    input::set_cursor(gui::state->input_state, gui::state->requested_cursor);
    gui::state->requested_cursor = nullptr;
}


void
gui::push_vertices(real32 *vertices, uint32 n_vertices)
{
    renderer::push_gui_vertices(vertices, n_vertices);
}


v2
gui::get_text_dimensions(fonts::FontAsset *font_asset, char const *str)
{
    // NOTE: This returns the dimensions around the main body of the text.
    // This does not include descenders.
    real32 line_height = fonts::font_unit_to_px(font_asset->height);
    real32 line_spacing = line_height * LINE_SPACING_FACTOR;
    real32 ascender = fonts::font_unit_to_px(font_asset->ascender);

    real32 start_x = 0.0f;
    real32 start_y = 0.0f - (line_height - ascender);
    real32 max_x = 0.0f;
    real32 curr_x = start_x;
    real32 curr_y = start_y;
    size_t str_length = strlen(str);

    for (uint32 idx = 0; idx < str_length; idx++) {
        char c = str[idx];

        if (c < 32) {
            if (c == '\n') {
                max_x = max(max_x, curr_x);
                curr_x = 0.0f;
                curr_y += line_spacing;
            }
            continue;
        }

        fonts::Character *character = font_asset->characters[c];

        if (!character) {
            logs::warning("Could not get character: %c", c);
            continue;
        }

        curr_x += fonts::frac_px_to_px(character->advance.x);
        curr_y += fonts::frac_px_to_px(character->advance.y);
    }

    max_x = max(max_x, curr_x);
    curr_y += line_height;

    return v2(max_x, curr_y);
}


v2
gui::center_bb(v2 container_position, v2 container_dimensions, v2 element_dimensions)
{
    return ceil(container_position + (container_dimensions / 2.0f) - (element_dimensions / 2.0f));
}


v2
gui::add_element_to_container(Container *container, v2 element_dimensions)
{
    // When adding a new element, we need to ensure we have enough space.
    //
    // We need:
    // * Enough space for the element itself.
    // * If this is not the first element, enough space for one
    //   `element_margin` on the main axis.
    //
    // On the main axis, we will allocate new space of this size.
    // On the orthogonal axis, we will ensure the element's dimensions are at
    // least this big, taking the container padding into account/
    //
    // For example, if we add a button that is 200x20 to a container which already
    // has buttons, we will add (20 + element_margin) to its height, and ensure
    // its width is at least (200 + padding).

    v2 new_element_position = container->next_element_position;
    v2 orthogonal_direction = v2(container->direction.y, container->direction.x);

    v2 required_space = element_dimensions;
    if (container->n_elements > 0) {
        required_space += (container->element_margin * container->direction);
    }

    auto d0 = (container->content_dimensions + required_space) * container->direction;
    auto d1 = max(container->content_dimensions, required_space) * orthogonal_direction;
    container->content_dimensions = d0 + d1;
    container->dimensions = container->content_dimensions +
        (container->padding * 2.0f) +
        v2(0.0f, container->title_bar_height);

    d0 = (container->dimensions - container->padding + container->element_margin) * container->direction;
    d1 = (container->padding + v2(0.0f, container->title_bar_height)) * orthogonal_direction;
    container->next_element_position = container->position + d0 + d1;

    container->n_elements++;

    return new_element_position;
}


void
gui::draw_rect(v2 position, v2 dimensions, v4 color)
{
    // NOTE: We use top-left as our origin, but OpenGL uses bottom-left.
    // Flip the y axis before drawing.
    real32 x0 = position.x;
    real32 x1 = x0 + dimensions.x;
    real32 y0 = (real32)gui::state->window_dimensions.y - position.y;
    real32 y1 = y0 - dimensions.y;

    real32 vertices[VERTEX_LENGTH * 6] = {
        x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
        x0, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
        x1, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
        x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
        x1, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
        x1, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a
    };
    push_vertices(vertices, 6);
}


void
gui::draw_text(
    char const *font_name, char const *str,
    v2 position,
    v4 color
) {
    fonts::FontAsset *font_asset = fonts::get_by_name(gui::state->font_assets, font_name);

    real32 line_height = fonts::font_unit_to_px(font_asset->height);
    real32 line_spacing = line_height * LINE_SPACING_FACTOR;
    real32 ascender = fonts::font_unit_to_px(font_asset->ascender);

    // NOTE: When changing this code, remember that the text positioning logic
    // needs to be replicated in `get_text_dimensions()`!
    real32 start_x = position.x;
    real32 start_y = position.y - (line_height - ascender);
    real32 curr_x = start_x;
    real32 curr_y = start_y;
    size_t str_length = strlen(str);

    for (uint32 idx = 0; idx < str_length; idx++) {
        char c = str[idx];

        if (c < 32) {
            if (c == '\n') {
                curr_x = start_x;
                curr_y += line_spacing;
            }
            continue;
        }

        fonts::Character *character = font_asset->characters[c];

        if (!character) {
            logs::warning("Could not get character: %c", c);
            continue;
        }

        real32 char_x = curr_x + character->bearing.x;
        real32 char_y = curr_y + fonts::font_unit_to_px(font_asset->height) - character->bearing.y;

        real32 tex_x = (real32)character->tex_coords.x / gui::state->texture_atlas_size.x;
        real32 tex_y = (real32)character->tex_coords.y / gui::state->texture_atlas_size.y;
        real32 tex_w = (real32)character->size.x / gui::state->texture_atlas_size.x;
        real32 tex_h = (real32)character->size.y / gui::state->texture_atlas_size.y;

        real32 w = (real32)character->size.x;
        real32 h = (real32)character->size.y;

        curr_x += fonts::frac_px_to_px(character->advance.x);
        curr_y += fonts::frac_px_to_px(character->advance.y);

        // Skip glyphs with no pixels, like spaces.
        if (w <= 0 || h <= 0) {
            continue;
        }

        // NOTE: We use top-left as our origin, but OpenGL uses bottom-left.
        // Flip the y axis before drawing.
        real32 x0 = char_x;
        real32 x1 = x0 + w;
        real32 y0 = (real32)gui::state->window_dimensions.y - char_y;
        real32 y1 = y0 - h;

        real32 tex_x0 = tex_x;
        real32 tex_x1 = tex_x0 + tex_w;
        real32 tex_y0 = tex_y;
        real32 tex_y1 = tex_y0 + tex_h;

        real32 vertices[VERTEX_LENGTH * 6] = {
            x0, y0, tex_x0, tex_y0, color.r, color.g, color.b, color.a,
            x0, y1, tex_x0, tex_y1, color.r, color.g, color.b, color.a,
            x1, y1, tex_x1, tex_y1, color.r, color.g, color.b, color.a,
            x0, y0, tex_x0, tex_y0, color.r, color.g, color.b, color.a,
            x1, y1, tex_x1, tex_y1, color.r, color.g, color.b, color.a,
            x1, y0, tex_x1, tex_y0, color.r, color.g, color.b, color.a
        };
        push_vertices(vertices, 6);
    }
}


void
gui::draw_text_shadow(
    char const *font_name, char const *str,
    v2 position,
    v4 color
) {
    draw_text(font_name, str, position + TEXT_SHADOW_OFFSET,
        v4(0.0f, 0.0f, 0.0f, color.a * 0.2f));
}


void
gui::draw_container(Container *container)
{
    draw_rect(container->position,
        v2(container->dimensions.x, container->title_bar_height),
        MAIN_DARKEN_COLOR);

    v2 text_dimensions = get_text_dimensions(fonts::get_by_name(gui::state->font_assets, "body"),
        container->title);
    v2 centered_text_position = center_bb(container->position,
        v2(container->dimensions.x, container->title_bar_height),
        text_dimensions);
    v2 text_position = v2(container->position.x + container->padding.x, centered_text_position.y);
    draw_text_shadow("body", container->title, text_position, LIGHT_TEXT_COLOR);
    draw_text("body", container->title, text_position, LIGHT_TEXT_COLOR);

    draw_rect(container->position + v2(0.0, container->title_bar_height),
        container->dimensions - v2(0.0, container->title_bar_height),
        WINDOW_BG_COLOR);
}


void
gui::draw_line(v2 start, v2 end, real32 thickness, v4 color)
{
    // NOTE: We use top-left as our origin, but OpenGL uses bottom-left.
    // Flip the y axis before drawing.
    v2 delta = normalize(end - start) * thickness;

    //    ----------->
    // 0------------------3
    // |                  |
    // 1------------------2
    real32 x0 = start.x + delta.y;
    real32 y0 = gui::state->window_dimensions.y - start.y;
    real32 x1 = start.x;
    real32 y1 = gui::state->window_dimensions.y - start.y - delta.x;
    real32 x2 = end.x;
    real32 y2 = gui::state->window_dimensions.y - end.y - delta.x;
    real32 x3 = end.x + delta.y;
    real32 y3 = gui::state->window_dimensions.y - end.y;

    real32 vertices[VERTEX_LENGTH * 6] = {
        x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
        x1, y1, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
        x2, y2, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
        x0, y0, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
        x2, y2, -1.0f, -1.0f, color.r, color.g, color.b, color.a,
        x3, y3, -1.0f, -1.0f, color.r, color.g, color.b, color.a
    };
    push_vertices(vertices, 6);
}


void
gui::draw_frame(v2 position, v2 bottomright, v2 thickness, v4 color)
{
    draw_line(v2(position.x, position.y),
        v2(bottomright.x, position.y),
        thickness.y, color);
    draw_line(v2(position.x, bottomright.y - thickness.y),
        v2(bottomright.x, bottomright.y - thickness.y),
        thickness.y, color);
    draw_line(v2(position.x, position.y),
        v2(position.x, bottomright.y),
        thickness.x, color);
    draw_line(v2(bottomright.x - thickness.x, position.y),
        v2(bottomright.x - thickness.x, bottomright.y),
        thickness.x, color);
}
