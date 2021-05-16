namespace cameras {
  // -----------------------------------------------------------
  // Types
  // -----------------------------------------------------------
  enum class CameraType {perspective, ortho};

  struct Camera {
    m4 view;
    v3 position;
    real64 pitch;
    m4 projection;
    m4 ui_projection;
    real32 exposure;
    real32 horizontal_fov;
    real32 vertical_fov;
    real32 near_clip_dist;
    real32 far_clip_dist;
    CameraType type;
    real64 yaw;
    v3 front;
    v3 up;
    real32 speed;
  };


  // -----------------------------------------------------------
  // Private functions
  // -----------------------------------------------------------
  void update_matrices_ortho(
    Camera *camera, uint32 window_width, uint32 window_height
  ) {
    if (window_width == 0 || window_height == 0) {
      return;
    }

    camera->view = glm::lookAt(
      camera->position,
      v3(0.0f, 0.0f, 0.0f),
      camera->up
    );

    camera->projection = glm::ortho(
      -10.0f, 10.0f, -10.0f, 10.0f,
      camera->near_clip_dist, camera->far_clip_dist
    );
  }


  void update_matrices_perspective(
    Camera *camera, uint32 window_width, uint32 window_height
  ) {
    if (window_width == 0 || window_height == 0) {
      return;
    }

    camera->front = normalize(v3(
      cos(radians(camera->yaw)) * cos(radians(camera->pitch)),
      -sin(radians(camera->pitch)),
      sin(radians(camera->yaw)) * cos(radians(camera->pitch))
    ));

    camera->view = glm::lookAt(
      camera->position, camera->position + camera->front, camera->up
    );

    camera->projection = glm::perspective(
      radians(camera->horizontal_fov),
      (real32)window_width / (real32)window_height,
      camera->near_clip_dist, camera->far_clip_dist
    );
    // https://en.wikipedia.org/wiki/Field_of_view_in_video_games#Field_of_view_calculations
    camera->vertical_fov = (real32)degrees(2 * atan(
      tan(radians(camera->horizontal_fov) / 2) * window_height / window_width
    ));
  }


  // -----------------------------------------------------------
  // Public functions
  // -----------------------------------------------------------
  void update_matrices(
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


  void update_ui_matrices(
    Camera *camera, uint32 window_width, uint32 window_height
  ) {
    camera->ui_projection = glm::ortho(
      0.0f, (real32)window_width, 0.0f, (real32)window_height
    );
  }


  void move_front_back(
    Camera *camera, real32 sign, real64 dt
  ) {
    camera->position += (sign * camera->speed * (real32)dt) * camera->front;
  }


  void move_left_right(
    Camera *camera, real32 sign, real64 dt
  ) {
    v3 direction = normalize(cross(
      camera->front, camera->up
    ));
    camera->position += (sign * camera->speed * (real32)dt) * direction;
  }


  void move_up_down(
    Camera *camera, real32 sign, real64 dt
  ) {
    camera->position += (sign * camera->speed * (real32)dt) * camera->up;
  }


  void update_mouse(
    Camera *camera, v2 mouse_offset
  ) {
    camera->yaw += mouse_offset.x;
    camera->pitch += mouse_offset.y;

    if (camera->pitch > 89.0f) {
      camera->pitch = 89.0f;
    } else if (camera->pitch < -89.0f) {
      camera->pitch = -89.0f;
    }
  }


  Camera* init(
    Camera *camera,
    CameraType new_type,
    uint32 window_width,
    uint32 window_height
  ) {
    camera->type = new_type;
    camera->yaw = -45.0f;
    camera->pitch = 0.0f;
    camera->position = v3(-7.0f, 3.0f, 7.0f);
    camera->front = v3(0.0f, 0.0f, 0.0f);
    camera->up = v3(0.0f, 1.0f, 0.0f);
    camera->speed = 5.0f;
    camera->horizontal_fov = 60.0f;
    camera->vertical_fov = 0.0f; // Filled in later
    camera->near_clip_dist = 0.1f;
    camera->far_clip_dist = 600.0f;
    camera->exposure = 1.0f;

    update_matrices(camera, window_width, window_height);
    update_ui_matrices(camera, window_width, window_height);

    return camera;
  }
}

using cameras::CameraType, cameras::Camera;