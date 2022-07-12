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
    static constexpr u32 GUI_MAX_N_VERTICES = 65536;
    static constexpr size_t GUI_VERTEX_SIZE = sizeof(f32) * gui::VERTEX_LENGTH;

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
        u32 g_buffer;
        mats::Texture *g_position_texture;
        mats::Texture *g_normal_texture;
        mats::Texture *g_albedo_texture;
        mats::Texture *g_pbr_texture;

        u32 l_buffer;
        mats::Texture *l_color_texture;
        mats::Texture *l_bright_color_texture;
        mats::Texture *l_depth_texture;

        u32 blur1_buffer;
        u32 blur2_buffer;
        mats::Texture *blur1_texture;
        mats::Texture *blur2_texture;

        u32 shadowmaps_3d_framebuffer;
        u32 shadowmaps_3d;
        mats::Texture *shadowmaps_3d_texture;
        u32 shadowmap_3d_width;
        u32 shadowmap_3d_height;

        u32 shadowmaps_2d_framebuffer;
        u32 shadowmaps_2d;
        mats::Texture *shadowmaps_2d_texture;
        u32 shadowmap_2d_width;
        u32 shadowmap_2d_height;

        f32 shadowmap_near_clip_dist;
        f32 shadowmap_far_clip_dist;
    };

    struct State {
        bool should_hide_ui;
        bool should_use_wireframe;
        mats::TextureType renderdebug_displayed_texture_type;
        shaders::Asset standard_depth_shader_asset;
        u32 ubo_shader_common;
        ShaderCommon shader_common;
        m4 shadowmap_3d_transforms[6 * MAX_N_LIGHTS];
        m4 shadowmap_2d_transforms[MAX_N_LIGHTS];
        BuiltinTextures builtin_textures;
        u32 gui_vao;
        u32 gui_vbo;
        mats::TextureAtlas gui_texture_atlas;
        shaders::Asset gui_shader_asset;
        Array<fonts::FontAsset> gui_font_assets;
        u32 gui_n_vertices_pushed;
    };

    static GLFWwindow * init_window(WindowSize *window_size);
    bool is_drawable_component_valid(drawable::Component *drawable_component);
    void destroy_drawable_component(drawable::Component *drawable_component);
    static void resize_renderer_buffers(
        memory::Pool *memory_pool,
        BuiltinTextures *builtin_textures,
        u32 width,
        u32 height
    );
    static void update_drawing_options(GLFWwindow *window);
    static void render(GLFWwindow *window, WindowSize *window_size);
    static void init(
        renderer::State *renderer_state,
        memory::Pool *memory_pool,
        u32 width,
        u32 height,
        GLFWwindow *window
    );
    static void start_drawing_gui();
    static void push_gui_vertices(f32 *vertices, u32 n_vertices);
    static void clear_gui_vertices();
    static void render_gui();
    static bool should_use_wireframe();
    static void set_should_use_wireframe(bool val);
    static renderer::BuiltinTextures * get_builtin_textures();
    static shaders::Asset * get_standard_depth_shader_asset();
    static iv2 get_gui_texture_atlas_size();
    static Array<fonts::FontAsset> * get_gui_font_assets();
    static void set_renderdebug_displayed_texture_type(mats::TextureType val);
    static bool should_hide_ui();
    static void set_should_hide_ui(bool val);

private:
    static void init_g_buffer(
        memory::Pool *memory_pool,
        u32 *g_buffer,
        mats::Texture **g_position_texture,
        mats::Texture **g_normal_texture,
        mats::Texture **g_albedo_texture,
        mats::Texture **g_pbr_texture,
        u32 width,
        u32 height
    );
    static void init_l_buffer(
        memory::Pool *memory_pool,
        u32 *l_buffer,
        mats::Texture **l_color_texture,
        mats::Texture **l_bright_color_texture,
        mats::Texture **l_depth_texture,
        u32 width,
        u32 height
    );
    static void init_blur_buffers(
        memory::Pool *memory_pool,
        u32 *blur1_buffer,
        u32 *blur2_buffer,
        mats::Texture **blur1_texture,
        mats::Texture **blur2_texture,
        u32 width,
        u32 height
    );
    static void init_ubo(u32 *ubo_shader_common);
    static void init_3d_shadowmaps(
        memory::Pool *memory_pool,
        u32 *shadowmaps_3d_framebuffer,
        u32 *shadowmaps_3d,
        mats::Texture **shadowmaps_3d_texture,
        u32 shadowmap_3d_width,
        u32 shadowmap_3d_height
    );
    static void init_2d_shadowmaps(
        memory::Pool *memory_pool,
        u32 *shadowmaps_2d_framebuffer,
        u32 *shadowmaps_2d,
        mats::Texture **shadowmaps_2d_texture,
        u32 shadowmap_2d_width,
        u32 shadowmap_2d_height
    );
    static void init_gui(memory::Pool *memory_pool);
    static void copy_scene_data_to_ubo(
        WindowSize *window_size,
        u32 current_shadow_light_idx,
        u32 current_shadow_light_type,
        bool is_blur_horizontal
    );
    static void draw(
        drawable::Mode render_mode,
        drawable::Component *drawable_component,
        mats::Material *material,
        m4 *model_matrix,
        m3 *model_normal_matrix,
        m4 *bone_matrices,
        shaders::Asset *standard_depth_shader_asset
    );
    static void draw_all(
        drawable::Pass render_pass,
        drawable::Mode render_mode,
        shaders::Asset *standard_depth_shader_asset
    );
    static void render_scene(drawable::Pass render_pass, drawable::Mode render_mode);

    static renderer::State *state;
};
