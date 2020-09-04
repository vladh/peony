#ifndef CONTROL_H
#define CONTROL_H

#include "types.hpp"

#include <glm/glm.hpp>


typedef struct Control {
  bool32 mouse_has_moved;
  real64 mouse_last_x;
  real64 mouse_last_y;
  real64 mouse_sensitivity;
} Control;

void control_init(Control *control);
glm::vec2 control_update(Control *control, real64 mouse_x, real64 mouse_y);

#endif
