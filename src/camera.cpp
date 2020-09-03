#include "types.hpp"
#include "memory.hpp"


void camera_update_matrix(State *state) {
  state->camera_front = glm::normalize(glm::vec3(
    cos(glm::radians(state->yaw)) * cos(glm::radians(state->pitch)),
    -sin(glm::radians(state->pitch)),
    sin(glm::radians(state->yaw)) * cos(glm::radians(state->pitch))
  ));
}

void camera_move_front_back(State *state, real32 sign) {
    state->camera_pos += sign * state->camera_speed * state->camera_front;
}

void camera_move_left_right(State *state, real32 sign) {
    glm::vec3 direction = glm::normalize(glm::cross(state->camera_front, state->camera_up));
    state->camera_pos += sign * direction * state->camera_speed;
}

void camera_move_up_down(State *state, real32 sign) {
    state->camera_pos += sign * state->camera_speed * state->camera_up;
}

void camera_update_mouse(State *state, real64 x, real64 y) {
  if (!state->mouse_has_moved) {
    state->mouse_last_x = x;
    state->mouse_last_y = y;
    state->mouse_has_moved = true;
  }

  real64 x_offset = x - state->mouse_last_x;
  real64 y_offset = y - state->mouse_last_y;
  state->mouse_last_x = x;
  state->mouse_last_y = y;

  x_offset *= state->mouse_sensitivity;
  y_offset *= state->mouse_sensitivity;

  state->yaw += x_offset;
  state->pitch += y_offset;

  // TODO: Do this in a better way.
  if (state->pitch > 89.0f) {
    state->pitch = 89.0f;
  } else if (state->pitch < -89.0f) {
    state->pitch = -89.0f;
  }

  camera_update_matrix(state);
}
