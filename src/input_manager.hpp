#ifndef CONTROL_H
#define CONTROL_H

struct InputManager {
  GLFWwindow *window;
  glm::vec2 mouse_pos;
  glm::vec2 mouse_3d_offset;
  real64 mouse_3d_sensitivity;
  bool32 key_states[1024];
  bool32 prev_key_states[1024];
  GLFWcursor *current_cursor;
  GLFWcursor *hand_cursor;
  bool32 have_ever_gotten_mouse_pos;

  InputManager(GLFWwindow *window);
  void update_mouse(glm::vec2 new_mouse_pos);
  void update_keys(int key, int scancode, int action, int mods);
  bool32 is_key_down(int key);
  bool32 is_key_up(int key);
  bool32 is_key_now_down(int key);
  bool32 is_key_now_up(int key);
  bool32 is_mouse_in_bb(
    real32 x0, real32 y0, real32 x1, real32 y1
  );
  void set_cursor(GLFWcursor *new_cursor);
};

#endif
