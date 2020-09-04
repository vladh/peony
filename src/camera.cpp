#include "types.hpp"
#include "memory.hpp"


void camera_init(Camera *camera) {
  camera->yaw = -90.0f;
  camera->pitch = 0.0f;
  camera->pos = glm::vec3(0.0f, 0.0f, 3.0f);
  camera->front = glm::vec3(0.0f, 0.0f, 0.0f);
  camera->up = glm::vec3(0.0f, 1.0f, 0.0f);
  camera->speed = 0.05f;
  camera->fov = 90.0f;
  camera->near = 0.1f;
  camera->far = 100.0f;
}

void camera_update_matrix(Camera *camera) {
  camera->front = glm::normalize(glm::vec3(
    cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch)),
    -sin(glm::radians(camera->pitch)),
    sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch))
  ));
}

void camera_move_front_back(Camera *camera, real32 sign) {
  camera->pos += sign * camera->speed * camera->front;
}

void camera_move_left_right(Camera *camera, real32 sign) {
  glm::vec3 direction = glm::normalize(glm::cross(
    camera->front, camera->up
  ));
  camera->pos += sign * direction * camera->speed;
}

void camera_move_up_down(Camera *camera, real32 sign) {
  camera->pos += sign * camera->speed * camera->up;
}

void camera_update_mouse(Camera *camera, glm::vec2 mouse_offset) {
  camera->yaw += mouse_offset.x;
  camera->pitch += mouse_offset.y;

  // TODO: Do this in a better way.
  if (camera->pitch > 89.0f) {
    camera->pitch = 89.0f;
  } else if (camera->pitch < -89.0f) {
    camera->pitch = -89.0f;
  }

  camera_update_matrix(camera);
}
