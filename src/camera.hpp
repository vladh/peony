#ifndef CAMERA_HPP
#define CAMERA_HPP

namespace Cameras {
  enum class CameraType {perspective, ortho};

  struct Camera {
    glm::mat4 view;
    glm::vec3 position;
    real64 pitch;
    glm::mat4 projection;
    glm::mat4 ui_projection;
    real32 exposure;
    real32 horizontal_fov;
    real32 vertical_fov;
    real32 near_clip_dist;
    real32 far_clip_dist;
    CameraType type;
    real64 yaw;
    glm::vec3 front;
    glm::vec3 up;
    real32 speed;
  };

  void move_front_back(
    Camera *camera, real32 sign, real64 dt
  );
  void move_left_right(
    Camera *camera, real32 sign, real64 dt
  );
  void move_up_down(
    Camera *camera, real32 sign, real64 dt
  );
  void update_mouse(
    Camera *camera, glm::vec2 mouse_offset
  );
  void update_matrices_perspective(
    Camera *camera, uint32 window_width, uint32 window_height
  );
  void update_matrices_ortho(
    Camera *camera, uint32 window_width, uint32 window_height
  );
  void update_matrices(
    Camera *camera, uint32 window_width, uint32 window_height
  );
  void update_ui_matrices(
    Camera *camera, uint32 window_width, uint32 window_height
  );
  Camera* init_camera(
    Camera *camera,
    CameraType new_type,
    uint32 window_width,
    uint32 window_height
  );
}

#endif
