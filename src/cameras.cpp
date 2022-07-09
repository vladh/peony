// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "cameras.hpp"
#include "intrinsics.hpp"


namespace cameras {
  pny_internal void update_matrices_ortho(
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


  pny_internal void update_matrices_perspective(
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
}


void cameras::update_matrices(
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


void cameras::update_ui_matrices(
  Camera *camera, uint32 window_width, uint32 window_height
) {
  camera->ui_projection = glm::ortho(
    0.0f, (real32)window_width, 0.0f, (real32)window_height
  );
}


void cameras::move_front_back(Camera *camera, real32 sign, real64 dt) {
  camera->position += (sign * camera->speed * (real32)dt) * camera->front;
}


void cameras::move_left_right(Camera *camera, real32 sign, real64 dt) {
  v3 direction = normalize(cross(
    camera->front, camera->up
  ));
  camera->position += (sign * camera->speed * (real32)dt) * direction;
}


void cameras::move_up_down(Camera *camera, real32 sign, real64 dt) {
  camera->position += (sign * camera->speed * (real32)dt) * camera->up;
}


void cameras::update_mouse(Camera *camera, v2 mouse_offset) {
  camera->yaw += mouse_offset.x;
  camera->pitch += mouse_offset.y;

  if (camera->pitch > 89.0f) {
    camera->pitch = 89.0f;
  } else if (camera->pitch < -89.0f) {
    camera->pitch = -89.0f;
  }
}


void cameras::init(
  CamerasState *cameras_state,
  uint32 window_width,
  uint32 window_height
) {
  cameras_state->camera_main.type = CameraType::perspective;
  cameras_state->camera_main.yaw = -45.0f;
  cameras_state->camera_main.pitch = 0.0f;
  cameras_state->camera_main.position = v3(-7.0f, 3.0f, 7.0f);
  cameras_state->camera_main.front = v3(0.0f, 0.0f, 0.0f);
  cameras_state->camera_main.up = v3(0.0f, 1.0f, 0.0f);
  cameras_state->camera_main.speed = 5.0f;
  cameras_state->camera_main.horizontal_fov = 60.0f;
  cameras_state->camera_main.vertical_fov = 0.0f; // Filled in later
  cameras_state->camera_main.near_clip_dist = 0.1f;
  cameras_state->camera_main.far_clip_dist = 600.0f;
  cameras_state->camera_main.exposure = 1.0f;

  update_matrices(&cameras_state->camera_main, window_width, window_height);
  update_ui_matrices(&cameras_state->camera_main, window_width, window_height);

  cameras_state->camera_active = &cameras_state->camera_main;
}
