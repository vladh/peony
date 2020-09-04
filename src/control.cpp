#include "control.hpp"

#include <glm/glm.hpp>


void control_init(Control *control) {
  control->mouse_has_moved = false;
  control->mouse_last_x = 0.0f;
  control->mouse_last_y = 0.0f;
  control->mouse_sensitivity = 0.1f;
}

glm::vec2 control_update(Control *control, real64 mouse_x, real64 mouse_y) {
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
