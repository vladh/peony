// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "../src_external/pstr.h"
#include "logs.hpp"
#include "input.hpp"


input::State *input::state = nullptr;


char *
input::get_text_input()
{
    return input::state->text_input;
}


bool
input::is_cursor_enabled()
{
    return input::state->is_cursor_enabled;
}


void
input::set_is_cursor_enabled(bool val)
{
    input::state->is_cursor_enabled = val;
}


bool
input::is_text_input_enabled()
{
    return input::state->is_text_input_enabled;
}


v2
input::get_mouse_offset()
{
    return input::state->mouse_offset;
}


v2
input::get_mouse_3d_offset()
{
    return input::state->mouse_3d_offset;
}


void
input::update_mouse_button(int button, int action, int mods)
{
    bool32 new_state = (action == GLFW_PRESS);
    if (new_state != input::state->mouse_button_states[button]) {
        input::state->mouse_button_states[button] = new_state;
        input::state->n_mouse_button_state_changes_this_frame[button]++;
    }
}


bool32
input::is_mouse_button_down(int button)
{
    return input::state->mouse_button_states[button];
}


bool32
input::is_mouse_button_up(int button)
{
    return !is_mouse_button_down(button);
}


bool32
input::is_mouse_button_now_down(int button)
{
    return is_mouse_button_down(button) &&
        input::state->n_mouse_button_state_changes_this_frame[button] > 0;
}


bool32
input::is_mouse_button_now_up(int button)
{
    return is_mouse_button_up(button) &&
        input::state->n_mouse_button_state_changes_this_frame[button] > 0;
}


void
input::update_mouse(v2 new_mouse_pos)
{
    if (!input::state->have_ever_gotten_mouse_pos) {
        input::state->mouse_pos = new_mouse_pos;
        input::state->have_ever_gotten_mouse_pos = true;
    }

    input::state->mouse_offset = new_mouse_pos - input::state->mouse_pos;
    input::state->mouse_3d_offset =
        input::state->mouse_offset * (real32)input::state->mouse_3d_sensitivity;
    input::state->mouse_pos = new_mouse_pos;
}


void
input::clear_text_input()
{
    pstr_clear(input::state->text_input);
}


void
input::enable_text_input()
{
    input::state->is_text_input_enabled = true;
    clear_text_input();
}


void
input::disable_text_input()
{
    input::state->is_text_input_enabled = false;
    clear_text_input();
}


void
input::do_text_input_backspace()
{
    size_t text_input_length = strlen(input::state->text_input);
    if (text_input_length == 0) {
        return;
    }
    input::state->text_input[text_input_length - 1] = '\0';
}


void
input::update_text_input(uint32 codepoint)
{
    if (!input::state->is_text_input_enabled) {
        return;
    }
    char poor_smashed_ascii_character = (char)codepoint;
    size_t text_input_length = strlen(input::state->text_input);

    if (text_input_length + 1 >= MAX_TEXT_INPUT_LENGTH - 1) {
        logs::error("Can't add another character to text_input, it's full.");
        return;
    }

    input::state->text_input[text_input_length] = poor_smashed_ascii_character;
    input::state->text_input[text_input_length + 1] = '\0';
}


void
input::update_keys(int key, int scancode, int action, int mods)
{
    bool32 new_state = (action == GLFW_PRESS || action == GLFW_REPEAT);
    if (new_state != input::state->key_states[key]) {
        input::state->key_states[key] = new_state;
        input::state->n_key_state_changes_this_frame[key]++;
    }
}


bool32
input::is_key_down(int key)
{
    return input::state->key_states[key];
}


bool32
input::is_key_up(int key)
{
    return !is_key_down(key);
}


bool32
input::is_key_now_down(int key)
{
    return is_key_down(key) && input::state->n_key_state_changes_this_frame[key] > 0;
}


bool32
input::is_key_now_up(int key)
{
    return is_key_up(key) && input::state->n_key_state_changes_this_frame[key] > 0;
}


bool32
input::is_mouse_in_bb(v2 topleft, v2 bottomright)
{
    return input::state->mouse_pos.x > topleft.x &&
        input::state->mouse_pos.x < bottomright.x &&
        input::state->mouse_pos.y > topleft.y &&
        input::state->mouse_pos.y < bottomright.y;
}


void
input::set_cursor(GLFWcursor *new_cursor)
{
    if (input::state->current_cursor == new_cursor) {
        return;
    }
    input::state->current_cursor = new_cursor;
    glfwSetCursor(input::state->window, new_cursor);
}


void
input::set_cursor(CursorType type)
{
    if (type == input::CursorType::none) {
        return;
    } else if (type == input::CursorType::arrow) {
        set_cursor(input::state->arrow_cursor);
    } else if (type == input::CursorType::ibeam) {
        set_cursor(input::state->ibeam_cursor);
    } else if (type == input::CursorType::crosshair) {
        set_cursor(input::state->crosshair_cursor);
    } else if (type == input::CursorType::hand) {
        set_cursor(input::state->hand_cursor);
    } else if (type == input::CursorType::hresize) {
        set_cursor(input::state->hresize_cursor);
    } else if (type == input::CursorType::vresize) {
        set_cursor(input::state->vresize_cursor);
    } else {
        logs::fatal("Unknown cursor type");
    }
}


void
input::reset_n_mouse_button_state_changes_this_frame()
{
    memset(input::state->n_mouse_button_state_changes_this_frame,
        0, sizeof(input::state->n_mouse_button_state_changes_this_frame));
}


void
input::reset_n_key_state_changes_this_frame()
{
    memset(input::state->n_key_state_changes_this_frame,
        0, sizeof(input::state->n_key_state_changes_this_frame));
}


void
input::init(input::State *input_state, GLFWwindow *window)
{
    input::state = input_state;
    input::state->window = window;
    input::state->mouse_3d_sensitivity = 0.1f;
    input::state->arrow_cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    input::state->ibeam_cursor = glfwCreateStandardCursor(GLFW_IBEAM_CURSOR);
    input::state->crosshair_cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
    input::state->hand_cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);
    input::state->hresize_cursor = glfwCreateStandardCursor(GLFW_HRESIZE_CURSOR);
    input::state->vresize_cursor = glfwCreateStandardCursor(GLFW_VRESIZE_CURSOR);
}
