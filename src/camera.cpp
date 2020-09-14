void camera_init(Camera *camera) {
  camera->yaw = -90.0f;
  camera->pitch = 0.0f;
  camera->position = glm::vec3(0.0f, 3.0f, 3.0f);
  camera->front = glm::vec3(0.0f, 0.0f, 0.0f);
  camera->up = glm::vec3(0.0f, 1.0f, 0.0f);
  camera->speed = 0.05f;
  camera->fov = 90.0f;
  camera->near_clip_dist = 0.1f;
  camera->far_clip_dist = 100.0f;
}

void camera_update_matrices(
  Camera *camera, real64 window_width, real64 window_height
) {
  camera->front = glm::normalize(glm::vec3(
    cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch)),
    -sin(glm::radians(camera->pitch)),
    sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch))
  ));

  camera->view = glm::lookAt(
    camera->position, camera->position + camera->front, camera->up
  );

  camera->projection = glm::perspective(
    glm::radians(camera->fov),
    (real32)window_width / (real32)window_height,
    camera->near_clip_dist, camera->far_clip_dist
  );
}

void camera_move_front_back(Camera *camera, real32 sign) {
  camera->position += sign * camera->speed * camera->front;
}

void camera_move_left_right(Camera *camera, real32 sign) {
  glm::vec3 direction = glm::normalize(glm::cross(
    camera->front, camera->up
  ));
  camera->position += sign * direction * camera->speed;
}

void camera_move_up_down(Camera *camera, real32 sign) {
  camera->position += sign * camera->speed * camera->up;
}

void camera_update_mouse(Camera *camera, glm::vec2 mouse_offset) {
  camera->yaw += mouse_offset.x;
  camera->pitch += mouse_offset.y;

  // TODO: Do this in a better way.
  // Do we still need this now that we're using quaternions?
  if (camera->pitch > 89.0f) {
    camera->pitch = 89.0f;
  } else if (camera->pitch < -89.0f) {
    camera->pitch = -89.0f;
  }
}
