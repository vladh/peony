void camera_init(Camera *camera, CameraType type) {
  camera->type = type;
  camera->yaw = -45.0f;
  camera->pitch = 0.0f;
  camera->position = glm::vec3(-10.0f, 3.0f, 10.0f);
  camera->front = glm::vec3(0.0f, 0.0f, 0.0f);
  camera->up = glm::vec3(0.0f, 1.0f, 0.0f);
  camera->speed = 0.05f;
  camera->fov = 90.0f;
  camera->near_clip_dist = 0.1f;
  camera->far_clip_dist = 100.0f;
  camera->exposure = 1.0f;
}

void camera_update_matrices_ortho(
  Camera *camera, real64 window_width, real64 window_height
) {
  camera->view = glm::lookAt(
    camera->position,
    glm::vec3(0.0f, 0.0f, 0.0f),
    camera->up
  );

  camera->projection = glm::ortho(
    -10.0f, 10.0f, -10.0f, 10.0f,
    camera->near_clip_dist, camera->far_clip_dist
  );
}

void camera_update_matrices_perspective(
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

void camera_update_matrices(
  Camera *camera, real64 window_width, real64 window_height
) {
  if (camera->type == CAMERA_PERSPECTIVE) {
    camera_update_matrices_perspective(camera, window_width, window_height);
  } else if (camera->type == CAMERA_ORTHO) {
    camera_update_matrices_ortho(camera, window_width, window_height);
  }
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

void camera_create_shadow_transforms(
  glm::mat4 shadow_transforms[6], glm::vec3 light_position,
  uint32 shadow_map_width, uint32 shadow_map_height,
  real32 near_clip_dist, real32 far_clip_dist
) {
  glm::mat4 shadow_projection = glm::perspective(
    glm::radians(90.0f),
    (real32)shadow_map_width / (real32)shadow_map_height,
    near_clip_dist, far_clip_dist
  );
  shadow_transforms[0] = shadow_projection * glm::lookAt(
    light_position,
    light_position + glm::vec3(1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f)
  );
  shadow_transforms[1] = shadow_projection * glm::lookAt(
    light_position,
    light_position + glm::vec3(-1.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, -1.0f, 0.0f)
  );
  shadow_transforms[2] = shadow_projection * glm::lookAt(
    light_position,
    light_position + glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 1.0f)
  );
  shadow_transforms[3] = shadow_projection * glm::lookAt(
    light_position,
    light_position + glm::vec3(0.0f, -1.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, -1.0f)
  );
  shadow_transforms[4] = shadow_projection * glm::lookAt(
    light_position,
    light_position + glm::vec3(0.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, -1.0f, 0.0f)
  );
  shadow_transforms[5] = shadow_projection * glm::lookAt(
    light_position,
    light_position + glm::vec3(0.0f, 0.0f, -1.0f),
    glm::vec3(0.0f, -1.0f, 0.0f)
  );
}
