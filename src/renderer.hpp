#pragma once

#include <GLFW/glfw3.h>
#include "types.hpp"
#include "materials.hpp"
#include "lights.hpp"
#include "memory.hpp"
#include "state.hpp"

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

using renderer::RendererState, renderer::ShaderCommon;
