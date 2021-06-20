/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#pragma once

#include "types.hpp"
#include "materials.hpp"
#include "lights.hpp"
#include "memory.hpp"
#include "cameras.hpp"
#include "gui.hpp"
#include "core.hpp"

struct EngineState;

namespace renderer {
  struct ShaderCommon {
    m4 view;
    m4 projection;
    m4 ui_projection;
    m4 shadowmap_3d_transforms[6 * MAX_N_LIGHTS];
    m4 shadowmap_2d_transforms[MAX_N_LIGHTS];

    v3 camera_position;
    float camera_pitch;

    float camera_horizontal_fov;
    float camera_vertical_fov;
    float camera_near_clip_dist;
    float camera_far_clip_dist;

    int n_point_lights;
    int n_directional_lights;
    int current_shadow_light_idx;
    int current_shadow_light_type;

    float shadow_far_clip_dist;
    bool is_blur_horizontal;
    TextureType renderdebug_displayed_texture_type;
    int unused_pad;

    float exposure;
    float t;
    int window_width;
    int window_height;

    v4 point_light_position[MAX_N_LIGHTS];
    v4 point_light_color[MAX_N_LIGHTS];
    v4 point_light_attenuation[MAX_N_LIGHTS];

    v4 directional_light_position[MAX_N_LIGHTS];
    v4 directional_light_direction[MAX_N_LIGHTS];
    v4 directional_light_color[MAX_N_LIGHTS];
    v4 directional_light_attenuation[MAX_N_LIGHTS];
  };

  struct BuiltinTextures {
    uint32 g_buffer;
    Texture *g_position_texture;
    Texture *g_normal_texture;
    Texture *g_albedo_texture;
    Texture *g_pbr_texture;

    uint32 l_buffer;
    Texture *l_color_texture;
    Texture *l_bright_color_texture;
    Texture *l_depth_texture;

    uint32 blur1_buffer;
    uint32 blur2_buffer;
    Texture *blur1_texture;
    Texture *blur2_texture;

    uint32 shadowmaps_3d_framebuffer;
    uint32 shadowmaps_3d;
    Texture *shadowmaps_3d_texture;
    uint32 shadowmap_3d_width;
    uint32 shadowmap_3d_height;

    uint32 shadowmaps_2d_framebuffer;
    uint32 shadowmaps_2d;
    Texture *shadowmaps_2d_texture;
    uint32 shadowmap_2d_width;
    uint32 shadowmap_2d_height;

    real32 shadowmap_near_clip_dist;
    real32 shadowmap_far_clip_dist;
  };

  struct RendererState {
    bool32 is_cursor_enabled;
    bool32 should_hide_ui;
    bool32 should_use_wireframe;
    TextureType renderdebug_displayed_texture_type;
    ShaderAsset standard_depth_shader_asset;
    uint32 ubo_shader_common;
    ShaderCommon shader_common;
    m4 shadowmap_3d_transforms[6 * MAX_N_LIGHTS];
    m4 shadowmap_2d_transforms[MAX_N_LIGHTS];
    BuiltinTextures builtin_textures;
  };

  void resize_renderer_buffers(
    MemoryPool *memory_pool,
    Array<Material> *materials,
    BuiltinTextures *builtin_textures,
    uint32 width,
    uint32 height
  );
  void update_drawing_options(
    RendererState *renderer_state,
    GLFWwindow *window
  );
  void render(
    RendererState *renderer_state,
    EngineState *engine_state,
    MaterialsState *materials_state,
    CamerasState *cameras_state,
    GuiState *gui_state,
    InputState *input_state,
    GLFWwindow *window,
    WindowSize *window_size
  );
  void init(
    RendererState *renderer_state,
    MemoryPool *memory_pool,
    uint32 width,
    uint32 height,
    GLFWwindow *window
  );
}

using renderer::RendererState, renderer::ShaderCommon, renderer::BuiltinTextures;
