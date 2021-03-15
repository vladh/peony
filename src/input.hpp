#ifndef INPUT_HPP
#define INPUT_HPP

struct InputState {
  GLFWwindow *window;
  glm::vec2 mouse_pos;
  glm::vec2 mouse_offset;
  glm::vec2 mouse_3d_offset;
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
};

namespace Input {
  void update_mouse_button(
    InputState *input_state, int button, int action, int mods
  );
  bool32 is_mouse_button_down(
    InputState *input_state, int button
  );
  bool32 is_mouse_button_up(
    InputState *input_state, int button
  );
  bool32 is_mouse_button_now_down(
    InputState *input_state, int button
  );
  bool32 is_mouse_button_now_up(
    InputState *input_state, int button
  );
  void update_mouse(
    InputState *input_state, glm::vec2 new_mouse_pos
  );
  void update_keys(
    InputState *input_state, int key, int scancode, int action, int mods
  );
  bool32 is_key_down(
    InputState *input_state, int key
  );
  bool32 is_key_up(
    InputState *input_state, int key
  );
  bool32 is_key_now_down(
    InputState *input_state, int key
  );
  bool32 is_key_now_up(
    InputState *input_state, int key
  );
  bool32 is_mouse_in_bb(
    InputState *input_state, glm::vec2 topleft, glm::vec2 bottomright
  );
  void set_cursor(
    InputState *input_state, GLFWcursor *new_cursor
  );
  void reset_n_mouse_button_state_changes_this_frame(
    InputState *input_state
  );
  void reset_n_key_state_changes_this_frame(
    InputState *input_state
  );
  InputState* init_input_state(
    InputState *input_state,
    GLFWwindow *window
  );
}

#endif
