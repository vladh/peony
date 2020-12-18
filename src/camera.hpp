#ifndef CAMERA_HPP
#define CAMERA_HPP

enum class CameraType {perspective, ortho};

class Camera {
public:
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

  Camera(
    CameraType new_type,
    uint32 window_width,
    uint32 window_height
  );
  void move_front_back(real32 sign, real64 dt);
  void move_left_right(real32 sign, real64 dt);
  void move_up_down(real32 sign, real64 dt);
  void update_mouse(glm::vec2 mouse_offset);
  void update_matrices_perspective(uint32 window_width, uint32 window_height);
  void update_matrices_ortho(uint32 window_width, uint32 window_height);
  void update_matrices(uint32 window_width, uint32 window_height);
  void update_ui_matrices(
    uint32 window_width, uint32 window_height
  );

private:
  CameraType type;
  real64 yaw;
  glm::vec3 front;
  glm::vec3 up;
  real32 speed;
};

#endif
