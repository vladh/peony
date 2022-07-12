// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"

class cameras {
public:
    enum class CameraType { perspective, ortho };

    struct Camera {
        m4 view;
        v3 position;
        f64 pitch;
        m4 projection;
        m4 ui_projection;
        f32 exposure;
        f32 horizontal_fov;
        f32 vertical_fov;
        f32 near_clip_dist;
        f32 far_clip_dist;
        CameraType type;
        f64 yaw;
        v3 front;
        v3 up;
        f32 speed;
    };

    struct State {
        Camera camera_main;
    };

    static Camera * get_main();
    static void update_matrices(
        Camera *camera, u32 window_width, u32 window_height
    );
    static void update_ui_matrices(
        Camera *camera, u32 window_width, u32 window_height
    );
    static void move_front_back(Camera *camera, f32 sign, f64 dt);
    static void move_left_right(Camera *camera, f32 sign, f64 dt);
    static void move_up_down(Camera *camera, f32 sign, f64 dt);
    static void update_mouse(Camera *camera, v2 mouse_offset);
    static void init(
        cameras::State *cameras_state,
        u32 window_width,
        u32 window_height
    );

private:
    static void update_matrices_ortho(
        Camera *camera, u32 window_width, u32 window_height
    );
    static void update_matrices_perspective(
        Camera *camera, u32 window_width, u32 window_height
    );

    static cameras::State *state;
};
