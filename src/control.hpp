#ifndef CONTROL_H
#define CONTROL_H

struct Control {
  bool32 mouse_has_moved;
  real64 mouse_last_x;
  real64 mouse_last_y;
  real64 mouse_sensitivity;
  bool32 key_states[1024];
  bool32 prev_key_states[1024];

  Control();
  glm::vec2 update_mouse(real64 mouse_x, real64 mouse_y);
  void update_keys(int key, int scancode, int action, int mods);
  bool32 is_key_down(int key);
  bool32 is_key_up(int key);
  bool32 is_key_now_down(int key);
  bool32 is_key_now_up(int key);
};

#endif
