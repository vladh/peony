#ifndef INPUT_MANAGER_HPP
#define INPUT_MANAGER_HPP

class InputManager {
public:
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

  void update_mouse_button(int button, int action, int mods);
  bool32 is_mouse_button_down(int button);
  bool32 is_mouse_button_up(int button);
  bool32 is_mouse_button_now_down(int button);
  bool32 is_mouse_button_now_up(int button);
  void update_mouse(glm::vec2 new_mouse_pos);
  void update_keys(int key, int scancode, int action, int mods);
  bool32 is_key_down(int key);
  bool32 is_key_up(int key);
  bool32 is_key_now_down(int key);
  bool32 is_key_now_up(int key);
  bool32 is_mouse_in_bb(glm::vec2 topleft, glm::vec2 bottomright);
  void set_cursor(GLFWcursor *new_cursor);
  void reset_n_mouse_button_state_changes_this_frame();
  void reset_n_key_state_changes_this_frame();
  InputManager(GLFWwindow *window);
};

#endif
