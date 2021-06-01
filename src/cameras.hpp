#pragma once

#include "types.hpp"

namespace cameras {
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

  struct CamerasState {
    Camera camera_main;
    Camera *camera_active;
  };

  void update_matrices(
    Camera *camera, uint32 window_width, uint32 window_height
  );
  void update_ui_matrices(
    Camera *camera, uint32 window_width, uint32 window_height
  );
  void move_front_back(Camera *camera, real32 sign, real64 dt);
  void move_left_right(Camera *camera, real32 sign, real64 dt);
  void move_up_down(Camera *camera, real32 sign, real64 dt);
  void update_mouse(Camera *camera, v2 mouse_offset);
  void init(
    CamerasState *cameras_state,
    uint32 window_width,
    uint32 window_height
  );
}

using cameras::CameraType, cameras::Camera, cameras::CamerasState;
