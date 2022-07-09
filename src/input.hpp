// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include <GLFW/glfw3.h>
#include "types.hpp"

namespace input {
  constexpr uint32 MAX_TEXT_INPUT_LENGTH = 512;
  constexpr uint32 MAX_TEXT_INPUT_COMMAND_LENGTH = 50;
  constexpr uint32 MAX_TEXT_INPUT_ARGUMENTS_LENGTH =
    MAX_TEXT_INPUT_LENGTH - MAX_TEXT_INPUT_COMMAND_LENGTH;

  struct InputState {
    GLFWwindow *window;
    bool is_cursor_enabled;
    v2 mouse_pos;
    v2 mouse_offset;
    v2 mouse_3d_offset;
    real64 mouse_3d_sensitivity;
    bool32 mouse_button_states[GLFW_MOUSE_BUTTON_LAST];
    uint32 n_mouse_button_state_changes_this_frame[GLFW_MOUSE_BUTTON_LAST];
    bool32 key_states[GLFW_KEY_LAST];
    uint32 n_key_state_changes_this_frame[GLFW_KEY_LAST];
    GLFWcursor *current_cursor;
    GLFWcursor *arrow_cursor;
    GLFWcursor *ibeam_cursor;
    GLFWcursor *crosshair_cursor;
    GLFWcursor *hand_cursor;
    GLFWcursor *hresize_cursor;
    GLFWcursor *vresize_cursor;
    bool32 have_ever_gotten_mouse_pos;
    bool32 is_text_input_enabled;
    char text_input[MAX_TEXT_INPUT_LENGTH];
  };

  void update_mouse_button(
    InputState *input_state, int button, int action, int mods
  );
  bool32 is_mouse_button_down(InputState *input_state, int button);
  bool32 is_mouse_button_up(InputState *input_state, int button);
  bool32 is_mouse_button_now_down(InputState *input_state, int button);
  bool32 is_mouse_button_now_up(InputState *input_state, int button);
  void update_mouse(InputState *input_state, v2 new_mouse_pos);
  void clear_text_input(InputState *input_state);
  void enable_text_input(InputState *input_state);
  void disable_text_input(InputState *input_state);
  void do_text_input_backspace(InputState *input_state);
  void update_text_input(InputState *input_state, uint32 codepoint);
  void update_keys(
    InputState *input_state, int key, int scancode, int action, int mods
  );
  bool32 is_key_down(InputState *input_state, int key);
  bool32 is_key_up(InputState *input_state, int key);
  bool32 is_key_now_down(InputState *input_state, int key);
  bool32 is_key_now_up(InputState *input_state, int key);
  bool32 is_mouse_in_bb(InputState *input_state, v2 topleft, v2 bottomright);
  void set_cursor(InputState *input_state, GLFWcursor *new_cursor);
  void reset_n_mouse_button_state_changes_this_frame(InputState *input_state);
  void reset_n_key_state_changes_this_frame(InputState *input_state);
  void init(InputState *input_state, GLFWwindow *window);
}

using input::InputState;
