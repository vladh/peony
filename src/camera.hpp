#ifndef CAMERA_H
#define CAMERA_H

enum CameraType {
  CAMERA_PERSPECTIVE, CAMERA_ORTHO
};

class Camera {
public:
  glm::mat4 view;
  glm::vec3 position;
  glm::mat4 projection;
  real32 exposure;

  Camera(CameraType type);
  void move_front_back(real32 sign);
  void move_left_right(real32 sign);
  void move_up_down(real32 sign);
  void update_mouse(glm::vec2 mouse_offset);
  void update_matrices(real64 window_width, real64 window_height);
  void update_matrices_perspective(real64 window_width, real64 window_height);
  void update_matrices_ortho(real64 window_width, real64 window_height);
  static void create_shadow_transforms(
    glm::mat4 shadow_transforms[6], glm::vec3 light_position,
    uint32 shadow_map_width, uint32 shadow_map_height,
    real32 near_clip_dist, real32 far_clip_dist
  );

private:
  CameraType type;
  real64 yaw;
  real64 pitch;
  glm::vec3 front;
  glm::vec3 up;
  real32 speed;
  real32 fov;
  real32 near_clip_dist;
  real32 far_clip_dist;
};


#endif
