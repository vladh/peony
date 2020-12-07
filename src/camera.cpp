Camera::Camera(
  CameraType new_type,
  uint32 window_width,
  uint32 window_height
) {
  this->type = new_type;
  this->yaw = -45.0f;
  this->pitch = 0.0f;
  this->position = glm::vec3(-7.0f, 3.0f, 7.0f);
  this->front = glm::vec3(0.0f, 0.0f, 0.0f);
  this->up = glm::vec3(0.0f, 1.0f, 0.0f);
  this->speed = 5.0f;
  this->horizontal_fov = 90.0f;
  this->vertical_fov = 0.0f; // Filled in later
  this->near_clip_dist = 0.1f;
  this->far_clip_dist = 100.0f;
  this->exposure = 1.0f;

  update_matrices(window_width, window_height);
  update_ui_matrices(window_width, window_height);
}


void Camera::update_matrices_ortho(
  uint32 window_width, uint32 window_height
) {
  if (window_width == 0 || window_height == 0) {
    return;
  }

  this->view = glm::lookAt(
    this->position,
    glm::vec3(0.0f, 0.0f, 0.0f),
    this->up
  );

  this->projection = glm::ortho(
    -10.0f, 10.0f, -10.0f, 10.0f,
    this->near_clip_dist, this->far_clip_dist
  );
}


void Camera::update_matrices_perspective(
  uint32 window_width, uint32 window_height
) {
  if (window_width == 0 || window_height == 0) {
    return;
  }

  this->front = glm::normalize(glm::vec3(
    cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch)),
    -sin(glm::radians(this->pitch)),
    sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch))
  ));

  this->view = glm::lookAt(
    this->position, this->position + this->front, this->up
  );

  this->projection = glm::perspective(
    glm::radians(this->horizontal_fov),
    (real32)window_width / (real32)window_height,
    this->near_clip_dist, this->far_clip_dist
  );
  // https://en.wikipedia.org/wiki/Field_of_view_in_video_games#Field_of_view_calculations
  this->vertical_fov = (real32)RAD_TO_DEG(2 * atan(
    tan(DEG_TO_RAD(this->horizontal_fov) / 2) * window_height / window_width
  ));
}


void Camera::update_matrices(
  uint32 window_width, uint32 window_height
) {
  if (window_width == 0 || window_height == 0) {
    return;
  }

  if (this->type == CameraType::perspective) {
    this->update_matrices_perspective(window_width, window_height);
  } else if (this->type == CameraType::ortho) {
    this->update_matrices_ortho(window_width, window_height);
  }
}


void Camera::update_ui_matrices(
  uint32 window_width, uint32 window_height
) {
  this->ui_projection = glm::ortho(
    0.0f, (real32)window_width, 0.0f, (real32)window_height
  );
}


void Camera::move_front_back(real32 sign, real64 dt) {
  this->position += (sign * this->speed * (real32)dt) * this->front;
}


void Camera::move_left_right(real32 sign, real64 dt) {
  glm::vec3 direction = glm::normalize(glm::cross(
    this->front, this->up
  ));
  this->position += (sign * this->speed * (real32)dt) * direction;
}


void Camera::move_up_down(real32 sign, real64 dt) {
  this->position += (sign * this->speed * (real32)dt) * this->up;
}


void Camera::update_mouse(glm::vec2 mouse_offset) {
  this->yaw += mouse_offset.x;
  this->pitch += mouse_offset.y;

  if (this->pitch > 89.0f) {
    this->pitch = 89.0f;
  } else if (this->pitch < -89.0f) {
    this->pitch = -89.0f;
  }
}


void Camera::create_cube_shadowmap_transforms(
  glm::mat4 cube_shadowmap_transforms[6 * MAX_N_LIGHTS],
  SpatialComponentManager *spatial_component_manager,
  LightComponentManager *light_component_manager,
  Array<EntityHandle> *point_lights,
  uint32 cube_shadowmap_width, uint32 cube_shadowmap_height,
  real32 near_clip_dist, real32 far_clip_dist
) {
  glm::mat4 perspective_projection = glm::perspective(
    glm::radians(90.0f),
    (real32)cube_shadowmap_width / (real32)cube_shadowmap_height,
    near_clip_dist, far_clip_dist
  );

  for (uint32 idx = 0; idx < point_lights->size; idx++) {
    EntityHandle light = *point_lights->get(idx);
    SpatialComponent *spatial_component = spatial_component_manager->get(light);
    LightComponent *light_component = light_component_manager->get(light);

    if (spatial_component && light_component) {
      glm::vec3 position = spatial_component->position;
      cube_shadowmap_transforms[(idx * 6) + 0] = perspective_projection * glm::lookAt(
        position,
        position + glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)
      );
      cube_shadowmap_transforms[(idx * 6) + 1] = perspective_projection * glm::lookAt(
        position,
        position + glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)
      );
      cube_shadowmap_transforms[(idx * 6) + 2] = perspective_projection * glm::lookAt(
        position,
        position + glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
      );
      cube_shadowmap_transforms[(idx * 6) + 3] = perspective_projection * glm::lookAt(
        position,
        position + glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f)
      );
      cube_shadowmap_transforms[(idx * 6) + 4] = perspective_projection * glm::lookAt(
        position,
        position + glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)
      );
      cube_shadowmap_transforms[(idx * 6) + 5] = perspective_projection * glm::lookAt(
        position,
        position + glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)
      );
    }
  }
}


void Camera::create_texture_shadowmap_transforms(
  glm::mat4 texture_shadowmap_transforms[MAX_N_LIGHTS],
  SpatialComponentManager *spatial_component_manager,
  LightComponentManager *light_component_manager,
  Array<EntityHandle> *directional_lights,
  uint32 texture_shadowmap_width, uint32 texture_shadowmap_height,
  real32 near_clip_dist, real32 far_clip_dist
) {
  real32 ortho_ratio = (real32)texture_shadowmap_width / (real32)texture_shadowmap_height;
  real32 ortho_width = 100.0f;
  real32 ortho_height = ortho_width / ortho_ratio;
  glm::mat4 ortho_projection = glm::ortho(
    -ortho_width, ortho_width,
    -ortho_height, ortho_height,
    near_clip_dist, far_clip_dist
  );

  for (uint32 idx = 0; idx < directional_lights->size; idx++) {
    EntityHandle light = *directional_lights->get(idx);
    SpatialComponent *spatial_component = spatial_component_manager->get(light);
    LightComponent *light_component = light_component_manager->get(light);

    if (light_component && spatial_component) {
      glm::vec3 position = spatial_component->position;
      texture_shadowmap_transforms[idx] = ortho_projection * glm::lookAt(
        position,
        position + light_component->direction,
        glm::vec3(0.0f, -1.0f, 0.0f)
      );
    }
  }
}
