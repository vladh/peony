// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "mats.hpp"
#include "lights.hpp"
#include "memory.hpp"
#include "cameras.hpp"
#include "gui.hpp"
#include "core.hpp"
#include "drawable.hpp"

struct EngineState;

class renderer {
public:
    static constexpr uint32 GUI_MAX_N_VERTICES = 65536;
    static constexpr size_t GUI_VERTEX_SIZE = sizeof(real32) * gui::VERTEX_LENGTH;

    struct WindowSize {
        int32 width; // in pixels (size of framebuffer)
        int32 height; // in pixels (size of framebuffer)
        uint32 screencoord_width; // in screen coordinates
        uint32 screencoord_height; // in screen coordinates
    };

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
        mats::TextureType renderdebug_displayed_texture_type;
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
        mats::Texture *g_position_texture;
        mats::Texture *g_normal_texture;
        mats::Texture *g_albedo_texture;
        mats::Texture *g_pbr_texture;

        uint32 l_buffer;
        mats::Texture *l_color_texture;
        mats::Texture *l_bright_color_texture;
        mats::Texture *l_depth_texture;

        uint32 blur1_buffer;
        uint32 blur2_buffer;
        mats::Texture *blur1_texture;
        mats::Texture *blur2_texture;

        uint32 shadowmaps_3d_framebuffer;
        uint32 shadowmaps_3d;
        mats::Texture *shadowmaps_3d_texture;
        uint32 shadowmap_3d_width;
        uint32 shadowmap_3d_height;

        uint32 shadowmaps_2d_framebuffer;
        uint32 shadowmaps_2d;
        mats::Texture *shadowmaps_2d_texture;
        uint32 shadowmap_2d_width;
        uint32 shadowmap_2d_height;

        real32 shadowmap_near_clip_dist;
        real32 shadowmap_far_clip_dist;
    };

    struct State {
        bool32 should_hide_ui;
        bool32 should_use_wireframe;
        mats::TextureType renderdebug_displayed_texture_type;
        shaders::Asset standard_depth_shader_asset;
        uint32 ubo_shader_common;
        ShaderCommon shader_common;
        m4 shadowmap_3d_transforms[6 * MAX_N_LIGHTS];
        m4 shadowmap_2d_transforms[MAX_N_LIGHTS];
        BuiltinTextures builtin_textures;
        uint32 gui_vao;
        uint32 gui_vbo;
        mats::TextureAtlas gui_texture_atlas;
        shaders::Asset gui_shader_asset;
        Array<fonts::FontAsset> gui_font_assets;
        uint32 gui_n_vertices_pushed;
    };

    static GLFWwindow * init_window(WindowSize *window_size);
    bool32 is_drawable_component_valid(drawable::Component *drawable_component);
    void destroy_drawable_component(drawable::Component *drawable_component);
    static void resize_renderer_buffers(
        MemoryPool *memory_pool,
        Array<mats::Material> *materials,
        BuiltinTextures *builtin_textures,
        uint32 width,
        uint32 height
    );
    static void update_drawing_options(InputState *input_state, GLFWwindow *window);
    static void render(
        EngineState *engine_state,
        mats::State *materials_state,
        CamerasState *cameras_state,
        InputState *input_state,
        GLFWwindow *window,
        WindowSize *window_size
    );
    static void init(
        renderer::State *renderer_state,
        InputState *input_state,
        MemoryPool *memory_pool,
        uint32 width,
        uint32 height,
        GLFWwindow *window
    );
    static void start_drawing_gui();
    static void render_gui();
    static void push_gui_vertices(f32 *vertices, u32 n_vertices);
    static void clear_gui_vertices();

private:
    static void init_g_buffer(
        MemoryPool *memory_pool,
        uint32 *g_buffer,
        mats::Texture **g_position_texture,
        mats::Texture **g_normal_texture,
        mats::Texture **g_albedo_texture,
        mats::Texture **g_pbr_texture,
        uint32 width,
        uint32 height
    );
    static void init_l_buffer(
        MemoryPool *memory_pool,
        uint32 *l_buffer,
        mats::Texture **l_color_texture,
        mats::Texture **l_bright_color_texture,
        mats::Texture **l_depth_texture,
        uint32 width,
        uint32 height
    );
    static void init_blur_buffers(
        MemoryPool *memory_pool,
        uint32 *blur1_buffer,
        uint32 *blur2_buffer,
        mats::Texture **blur1_texture,
        mats::Texture **blur2_texture,
        uint32 width,
        uint32 height
    );
    static void init_ubo(uint32 *ubo_shader_common);
    static void init_3d_shadowmaps(
        MemoryPool *memory_pool,
        uint32 *shadowmaps_3d_framebuffer,
        uint32 *shadowmaps_3d,
        mats::Texture **shadowmaps_3d_texture,
        uint32 shadowmap_3d_width,
        uint32 shadowmap_3d_height
    );
    static void init_2d_shadowmaps(
        MemoryPool *memory_pool,
        uint32 *shadowmaps_2d_framebuffer,
        uint32 *shadowmaps_2d,
        mats::Texture **shadowmaps_2d_texture,
        uint32 shadowmap_2d_width,
        uint32 shadowmap_2d_height
    );
    static void init_gui(MemoryPool *memory_pool);
    static void copy_scene_data_to_ubo(
        CamerasState *cameras_state,
        EngineState *engine_state,
        WindowSize *window_size,
        uint32 current_shadow_light_idx,
        uint32 current_shadow_light_type,
        bool32 is_blur_horizontal
    );
    static void draw(
        drawable::Mode render_mode,
        drawable::ComponentSet *drawable_component_set,
        drawable::Component *drawable_component,
        mats::Material *material,
        m4 *model_matrix,
        m3 *model_normal_matrix,
        m4 *bone_matrices,
        shaders::Asset *standard_depth_shader_asset
    );
    static void draw_all(
        EntitySet *entity_set,
        drawable::ComponentSet *drawable_component_set,
        SpatialComponentSet *spatial_component_set,
        AnimationComponentSet *animation_component_set,
        Array<mats::Material> *materials,
        drawable::Pass render_pass,
        drawable::Mode render_mode,
        shaders::Asset *standard_depth_shader_asset
    );
    static void render_scene(
        EngineState *engine_state,
        mats::State *materials_state,
        drawable::Pass render_pass,
        drawable::Mode render_mode
    );

    static renderer::State *state;
};
