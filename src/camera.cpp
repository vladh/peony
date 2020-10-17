Camera::Camera(CameraType new_type) {
  this->type = new_type;
  this->yaw = -45.0f;
  this->pitch = 0.0f;
  this->position = glm::vec3(-7.0f, 3.0f, 7.0f);
  this->front = glm::vec3(0.0f, 0.0f, 0.0f);
  this->up = glm::vec3(0.0f, 1.0f, 0.0f);
  this->speed = 5.0f;
  this->fov = 90.0f;
  this->near_clip_dist = 0.1f;
  this->far_clip_dist = 100.0f;
  this->exposure = 1.0f;
}

void Camera::update_matrices_ortho(
  real64 window_width, real64 window_height
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
  real64 window_width, real64 window_height
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
    glm::radians(this->fov),
    (real32)window_width / (real32)window_height,
    this->near_clip_dist, this->far_clip_dist
  );
}

void Camera::update_matrices(
  real64 window_width, real64 window_height
) {
  if (window_width == 0 || window_height == 0) {
    return;
  }

  if (this->type == CAMERA_PERSPECTIVE) {
    this->update_matrices_perspective(window_width, window_height);
  } else if (this->type == CAMERA_ORTHO) {
    this->update_matrices_ortho(window_width, window_height);
  }
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

  // TODO: Do this in a better way.
  // Do we still need this now that we're using quaternions?
  if (this->pitch > 89.0f) {
    this->pitch = 89.0f;
  } else if (this->pitch < -89.0f) {
    this->pitch = -89.0f;
  }
}

void Camera::create_shadow_transforms(
  glm::mat4 shadow_transforms[6 * MAX_N_LIGHTS],
  SpatialComponentManager *spatial_component_manager,
  LightComponentManager *light_component_manager,
  Array<EntityHandle> *lights,
  uint32 shadowmap_width, uint32 shadowmap_height,
  real32 near_clip_dist, real32 far_clip_dist
) {
  glm::mat4 perspective_projection = glm::perspective(
    glm::radians(90.0f),
    (real32)shadowmap_width / (real32)shadowmap_height,
    near_clip_dist, far_clip_dist
  );

  glm::mat4 ortho_projection = glm::ortho(
    -10.0f, 10.0f, -10.0f, 10.0f, near_clip_dist, far_clip_dist
  );

  for (uint32 idx = 0; idx < lights->size; idx++) {
    EntityHandle light = *lights->get(idx);
    SpatialComponent *spatial_component = spatial_component_manager->get(light);
    LightComponent *light_component = light_component_manager->get(light);
    glm::vec3 pos = spatial_component->position;

    if (light_component->direction == glm::vec3(0.0f, 0.0f, 0.0f)) {
      shadow_transforms[(idx * 6) + 0] = perspective_projection * glm::lookAt(
        pos,
        pos + glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)
      );
      shadow_transforms[(idx * 6) + 1] = perspective_projection * glm::lookAt(
        pos,
        pos + glm::vec3(-1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)
      );
      shadow_transforms[(idx * 6) + 2] = perspective_projection * glm::lookAt(
        pos,
        pos + glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
      );
      shadow_transforms[(idx * 6) + 3] = perspective_projection * glm::lookAt(
        pos,
        pos + glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f)
      );
      shadow_transforms[(idx * 6) + 4] = perspective_projection * glm::lookAt(
        pos,
        pos + glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)
      );
      shadow_transforms[(idx * 6) + 5] = perspective_projection * glm::lookAt(
        pos,
        pos + glm::vec3(0.0f, 0.0f, -1.0f),
        glm::vec3(0.0f, -1.0f, 0.0f)
      );
    } else {
      shadow_transforms[(idx * 6) + 0] = ortho_projection * glm::lookAt(
        pos,
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
      );
    }
  }
}
