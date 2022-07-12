// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include <GLFW/glfw3.h>
#include "types.hpp"

class input {
public:
    static constexpr u32 MAX_TEXT_INPUT_LENGTH = 512;
    static constexpr u32 MAX_TEXT_INPUT_COMMAND_LENGTH = 50;
    static constexpr u32 MAX_TEXT_INPUT_ARGUMENTS_LENGTH =
        MAX_TEXT_INPUT_LENGTH - MAX_TEXT_INPUT_COMMAND_LENGTH;

    enum class CursorType {
        none,
        arrow,
        ibeam,
        crosshair,
        hand,
        hresize,
        vresize,
    };

    struct State {
        GLFWwindow *window;
        bool is_cursor_enabled;
        v2 mouse_pos;
        v2 mouse_offset;
        v2 mouse_3d_offset;
        f64 mouse_3d_sensitivity;
        bool mouse_button_states[GLFW_MOUSE_BUTTON_LAST];
        u32 n_mouse_button_state_changes_this_frame[GLFW_MOUSE_BUTTON_LAST];
        bool key_states[GLFW_KEY_LAST];
        u32 n_key_state_changes_this_frame[GLFW_KEY_LAST];
        GLFWcursor *current_cursor;
        GLFWcursor *arrow_cursor;
        GLFWcursor *ibeam_cursor;
        GLFWcursor *crosshair_cursor;
        GLFWcursor *hand_cursor;
        GLFWcursor *hresize_cursor;
        GLFWcursor *vresize_cursor;
        bool have_ever_gotten_mouse_pos;
        bool is_text_input_enabled;
        char text_input[MAX_TEXT_INPUT_LENGTH];
    };

    static char * get_text_input();
    static bool is_cursor_enabled();
    static void set_is_cursor_enabled(bool val);
    static bool is_text_input_enabled();
    static v2 get_mouse_offset();
    static v2 get_mouse_3d_offset();
    static void update_mouse_button(int button, int action, int mods);
    static bool is_mouse_button_down(int button);
    static bool is_mouse_button_up(int button);
    static bool is_mouse_button_now_down(int button);
    static bool is_mouse_button_now_up(int button);
    static void update_mouse(v2 new_mouse_pos);
    static void clear_text_input();
    static void enable_text_input();
    static void disable_text_input();
    static void do_text_input_backspace();
    static void update_text_input(u32 codepoint);
    static void update_keys(int key, int scancode, int action, int mods);
    static bool is_key_down(int key);
    static bool is_key_up(int key);
    static bool is_key_now_down(int key);
    static bool is_key_now_up(int key);
    static bool is_mouse_in_bb(v2 topleft, v2 bottomright);
    static void set_cursor(GLFWcursor *new_cursor);
    static void set_cursor(CursorType type);
    static void reset_n_mouse_button_state_changes_this_frame();
    static void reset_n_key_state_changes_this_frame();
    static void init(input::State *state, GLFWwindow *window);

private:
    static input::State *state;
};
