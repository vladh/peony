#ifndef CAMERA_H
#define CAMERA_H

enum CameraType {
  CAMERA_PERSPECTIVE, CAMERA_ORTHO
};

class Camera {
public:
  glm::mat4 view;
  glm::vec3 position;
  real64 pitch;
  glm::mat4 projection;
  real32 exposure;
  real32 horizontal_fov;
  real32 vertical_fov;
  real32 near_clip_dist;
  real32 far_clip_dist;

  Camera(CameraType type);
  void move_front_back(real32 sign, real64 dt);
  void move_left_right(real32 sign, real64 dt);
  void move_up_down(real32 sign, real64 dt);
  void update_mouse(glm::vec2 mouse_offset);
  void update_matrices(real64 window_width, real64 window_height);
  void update_matrices_perspective(real64 window_width, real64 window_height);
  void update_matrices_ortho(real64 window_width, real64 window_height);
  static void create_shadow_transforms(
    glm::mat4 shadow_transforms[6 * MAX_N_LIGHTS],
    SpatialComponentManager *spatial_component_manager,
    LightComponentManager *light_component_manager,
    Array<EntityHandle> *lights,
    uint32 cube_shadowmap_width, uint32 cube_shadowmap_height,
    uint32 texture_shadowmap_width, uint32 texture_shadowmap_height,
    real32 near_clip_dist, real32 far_clip_dist
  );

private:
  CameraType type;
  real64 yaw;
  glm::vec3 front;
  glm::vec3 up;
  real32 speed;
};


#endif
