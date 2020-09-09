#ifndef CONTROL_H
#define CONTROL_H

#include "types.hpp"

#include <glm/glm.hpp>


struct Control {
  bool32 mouse_has_moved;
  real64 mouse_last_x;
  real64 mouse_last_y;
  real64 mouse_sensitivity;
  bool32 key_states[1024];
  bool32 prev_key_states[1024];
};

void control_init(Control *control);
glm::vec2 control_update_mouse(Control *control, real64 mouse_x, real64 mouse_y);
void control_update_keys(
  Control *control, int key, int scancode, int action, int mods
);
bool32 control_is_key_down(Control *control, int key);
bool32 control_is_key_up(Control *control, int key);
bool32 control_is_key_now_down(Control *control, int key);
bool32 control_is_key_now_up(Control *control, int key);

#endif
