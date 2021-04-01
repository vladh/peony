void Cameras::update_matrices_ortho(
  Camera *camera, uint32 window_width, uint32 window_height
) {
  if (window_width == 0 || window_height == 0) {
    return;
  }

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


void Cameras::update_matrices_perspective(
  Camera *camera, uint32 window_width, uint32 window_height
) {
  if (window_width == 0 || window_height == 0) {
    return;
  }

  camera->front = glm::normalize(glm::vec3(
    cos(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch)),
    -sin(glm::radians(camera->pitch)),
    sin(glm::radians(camera->yaw)) * cos(glm::radians(camera->pitch))
  ));

  camera->view = glm::lookAt(
    camera->position, camera->position + camera->front, camera->up
  );

  camera->projection = glm::perspective(
    glm::radians(camera->horizontal_fov),
    (real32)window_width / (real32)window_height,
    camera->near_clip_dist, camera->far_clip_dist
  );
  // https://en.wikipedia.org/wiki/Field_of_view_in_video_games#Field_of_view_calculations
  camera->vertical_fov = (real32)RAD_TO_DEG(2 * atan(
    tan(DEG_TO_RAD(camera->horizontal_fov) / 2) * window_height / window_width
  ));
}


void Cameras::update_matrices(
  Camera *camera, uint32 window_width, uint32 window_height
) {
  if (window_width == 0 || window_height == 0) {
    return;
  }

  if (camera->type == CameraType::perspective) {
    update_matrices_perspective(camera, window_width, window_height);
  } else if (camera->type == CameraType::ortho) {
    update_matrices_ortho(camera, window_width, window_height);
  }
}


void Cameras::update_ui_matrices(
  Camera *camera, uint32 window_width, uint32 window_height
) {
  camera->ui_projection = glm::ortho(
    0.0f, (real32)window_width, 0.0f, (real32)window_height
  );
}


void Cameras::move_front_back(
  Camera *camera, real32 sign, real64 dt
) {
  camera->position += (sign * camera->speed * (real32)dt) * camera->front;
}


void Cameras::move_left_right(
  Camera *camera, real32 sign, real64 dt
) {
  glm::vec3 direction = glm::normalize(glm::cross(
    camera->front, camera->up
  ));
  camera->position += (sign * camera->speed * (real32)dt) * direction;
}


void Cameras::move_up_down(
  Camera *camera, real32 sign, real64 dt
) {
  camera->position += (sign * camera->speed * (real32)dt) * camera->up;
}


void Cameras::update_mouse(
  Camera *camera, glm::vec2 mouse_offset
) {
  camera->yaw += mouse_offset.x;
  camera->pitch += mouse_offset.y;

  if (camera->pitch > 89.0f) {
    camera->pitch = 89.0f;
  } else if (camera->pitch < -89.0f) {
    camera->pitch = -89.0f;
  }
}


Camera* Cameras::init_camera(
  Camera *camera,
  CameraType new_type,
  uint32 window_width,
  uint32 window_height
) {
  camera->type = new_type;
  camera->yaw = -45.0f;
  camera->pitch = 0.0f;
  camera->position = glm::vec3(-7.0f, 3.0f, 7.0f);
  camera->front = glm::vec3(0.0f, 0.0f, 0.0f);
  camera->up = glm::vec3(0.0f, 1.0f, 0.0f);
  camera->speed = 5.0f;
  camera->horizontal_fov = 60.0f;
  camera->vertical_fov = 0.0f; // Filled in later
  camera->near_clip_dist = 0.1f;
  camera->far_clip_dist = 100.0f;
  camera->exposure = 1.0f;

  update_matrices(camera, window_width, window_height);
  update_ui_matrices(camera, window_width, window_height);

  return camera;
}
