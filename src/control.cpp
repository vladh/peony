#include "control.hpp"

#include <glm/glm.hpp>
#include <GLFW/glfw3.h>


void control_init(Control *control) {
  control->mouse_has_moved = false;
  control->mouse_last_x = 0.0f;
  control->mouse_last_y = 0.0f;
  control->mouse_sensitivity = 0.1f;

  for (uint32 idx = 0; idx < len(control->key_states); idx++) {
    control->key_states[idx] = false;
    control->prev_key_states[idx] = true;
  }
}

glm::vec2 control_update_mouse(Control *control, real64 mouse_x, real64 mouse_y) {
  if (!control->mouse_has_moved) {
    control->mouse_last_x = mouse_x;
    control->mouse_last_y = mouse_y;
    control->mouse_has_moved = true;
  }

  glm::vec2 mouse_offset;
  mouse_offset.x = (real32)(mouse_x - control->mouse_last_x);
  mouse_offset.y = (real32)(mouse_y - control->mouse_last_y);
  control->mouse_last_x = mouse_x;
  control->mouse_last_y = mouse_y;

  mouse_offset.x *= (real32)control->mouse_sensitivity;
  mouse_offset.y *= (real32)control->mouse_sensitivity;

  return mouse_offset;
}

void control_update_keys(
  Control *control, int key, int scancode, int action, int mods
) {
  control->prev_key_states[key] = control->key_states[key];
  if (action == GLFW_PRESS) {
    control->key_states[key] = true;
  } else if (action == GLFW_RELEASE) {
    control->key_states[key] = false;
  }
}

bool32 control_is_key_down(Control *control, int key) {
  return control->key_states[key];
}

bool32 control_is_key_up(Control *control, int key) {
  return !control->key_states[key];
}

bool32 control_is_key_now_down(Control *control, int key) {
  return control->key_states[key] && !control->prev_key_states[key];
}

bool32 control_is_key_now_up(Control *control, int key) {
  return !control->key_states[key] && control->prev_key_states[key];
}
