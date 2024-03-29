// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "cameras.hpp"
#include "intrinsics.hpp"


cameras::State *cameras::state = nullptr;


cameras::Camera *
cameras::get_main()
{
    return &cameras::state->camera_main;
}


void
cameras::update_matrices(Camera *camera)
{
    WindowSize *window_size = core::get_window_size();
    if (window_size->width == 0 || window_size->height == 0) {
        return;
    }

    if (camera->type == CameraType::perspective) {
        update_matrices_perspective(camera, window_size->width, window_size->height);
    } else if (camera->type == CameraType::ortho) {
        update_matrices_ortho(camera, window_size->width, window_size->height);
    }
}


void
cameras::update_ui_matrices(Camera *camera)
{
    WindowSize *window_size = core::get_window_size();
    camera->ui_projection = glm::ortho(0.0f, (f32)window_size->width,
        0.0f, (f32)window_size->height);
}


void
cameras::move_front_back(Camera *camera, f32 sign, f64 dt)
{
    camera->position += (sign * camera->speed * (f32)dt) * camera->front;
}


void
cameras::move_left_right(Camera *camera, f32 sign, f64 dt)
{
    v3 direction = normalize(cross(
            camera->front, camera->up
    ));
    camera->position += (sign * camera->speed * (f32)dt) * direction;
}


void
cameras::move_up_down(Camera *camera, f32 sign, f64 dt)
{
    camera->position += (sign * camera->speed * (f32)dt) * camera->up;
}


void
cameras::update_mouse(Camera *camera, v2 mouse_offset)
{
    camera->yaw += mouse_offset.x;
    camera->pitch += mouse_offset.y;

    if (camera->pitch > 89.0f) {
        camera->pitch = 89.0f;
    } else if (camera->pitch < -89.0f) {
        camera->pitch = -89.0f;
    }
}


void
cameras::init(
    cameras::State *cameras_state,
    u32 window_width,
    u32 window_height
) {
    cameras::state = cameras_state;
    cameras::state->camera_main.type = CameraType::perspective;
    cameras::state->camera_main.yaw = -45.0f;
    cameras::state->camera_main.pitch = 0.0f;
    cameras::state->camera_main.position = v3(-7.0f, 3.0f, 7.0f);
    cameras::state->camera_main.front = v3(0.0f, 0.0f, 0.0f);
    cameras::state->camera_main.up = v3(0.0f, 1.0f, 0.0f);
    cameras::state->camera_main.speed = 5.0f;
    cameras::state->camera_main.horizontal_fov = 60.0f;
    cameras::state->camera_main.vertical_fov = 0.0f; // Filled in later
    cameras::state->camera_main.near_clip_dist = 0.1f;
    cameras::state->camera_main.far_clip_dist = 600.0f;
    cameras::state->camera_main.exposure = 1.0f;

    update_matrices(&cameras::state->camera_main);
    update_ui_matrices(&cameras::state->camera_main);
}


void
cameras::update_matrices_ortho(
    Camera *camera, u32 window_width, u32 window_height
) {
    if (window_width == 0 || window_height == 0) {
        return;
    }

    camera->view = glm::lookAt(camera->position, v3(0.0f, 0.0f, 0.0f), camera->up);

    camera->projection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f,
        camera->near_clip_dist, camera->far_clip_dist);
}


void
cameras::update_matrices_perspective(
    Camera *camera, u32 window_width, u32 window_height
) {
    if (window_width == 0 || window_height == 0) {
        return;
    }

    camera->front = normalize(v3(
            cos(radians(camera->yaw)) * cos(radians(camera->pitch)),
            -sin(radians(camera->pitch)),
            sin(radians(camera->yaw)) * cos(radians(camera->pitch))));

    camera->view = glm::lookAt(camera->position,
        camera->position + camera->front, camera->up);

    camera->projection = glm::perspective(radians(camera->horizontal_fov),
        (f32)window_width / (f32)window_height,
        camera->near_clip_dist, camera->far_clip_dist);
    // https://en.wikipedia.org/wiki/Field_of_view_in_video_games#Field_of_view_calculations
    camera->vertical_fov = (f32)degrees(2 *
        atan(tan(radians(camera->horizontal_fov) / 2) * window_height / window_width));
}
