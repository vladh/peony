#ifndef OPENGL_HPP
#define OPENGL_HPP

constexpr bool32 USE_OPENGL_DEBUG = false;
constexpr uint16 MAX_N_LIGHTS = 8;

struct WindowInfo;

enum class RenderMode {regular, depth};

typedef uint32 RenderPassFlag;

namespace RenderPass {
  RenderPassFlag none = 0;
  RenderPassFlag shadowcaster = (1 << 0);
  RenderPassFlag deferred = (1 << 1);
  RenderPassFlag forward_depth = (1 << 2);
  RenderPassFlag forward_nodepth = (1 << 3);
  RenderPassFlag forward_skybox = (1 << 4);
  RenderPassFlag lighting = (1 << 5);
  RenderPassFlag postprocessing = (1 << 6);
  RenderPassFlag preblur = (1 << 7);
  RenderPassFlag blur1 = (1 << 8);
  RenderPassFlag blur2 = (1 << 9);
  RenderPassFlag renderdebug = (1 << 10);
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

namespace Renderer {
  // The position is used in positioning the shadow map, but not
  // in the light calculations. We need to scale the position
  // by some factor such that the shadow map covers the biggest
  // possible area.
  constexpr v3 DIRECTIONAL_LIGHT_DISTANCE = v3(75.0f, 15.0f, 75.0f);

  const char* render_pass_to_string(RenderPassFlag render_pass);
  RenderPassFlag render_pass_from_string(const char* str);
  void resize_renderer_buffers(
    MemoryPool *memory_pool,
    Array<Material> *materials,
    BuiltinTextures *builtin_textures,
    uint32 width,
    uint32 height
  );
  void init_ubo(State *state);
  void init_shadowmaps(
    MemoryPool *memory_pool,
    BuiltinTextures *builtin_textures
  );
  void init_g_buffer(
    MemoryPool *memory_pool,
    BuiltinTextures *builtin_textures,
    uint32 width,
    uint32 height
  );
  void init_l_buffer(
    MemoryPool *memory_pool,
    BuiltinTextures *builtin_textures,
    uint32 width,
    uint32 height
  );
  void init_blur_buffers(
    MemoryPool *memory_pool,
    BuiltinTextures *builtin_textures,
    uint32 width,
    uint32 height
  );
  void update_drawing_options(State *state, GLFWwindow *window);
  void copy_scene_data_to_ubo(
    State *state,
    uint32 current_shadow_light_idx,
    uint32 current_shadow_light_type,
    bool32 is_blur_horizontal
  );
  void copy_scene_data_to_ubo(State *state);
  void init_window(WindowInfo *window_info);
  void destroy_window();
  void framebuffer_size_callback(GLFWwindow* window, int width, int height);
  void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
  void mouse_callback(GLFWwindow *window, real64 x, real64 y);
  void key_callback(
    GLFWwindow* window, int key, int scancode, int action, int mods
  );
  void char_callback(
    GLFWwindow* window, uint32 codepoint
  );
  void reload_shaders(State *state);
  void render_scene(
    State *state,
    RenderPassFlag render_pass,
    RenderMode render_mode
  );
  void set_heading(
    State *state,
    const char *text, real32 opacity,
    real32 fadeout_duration, real32 fadeout_delay
  );
  void render_scene_ui(
    State *state
  );
  void render(State *state);
}

#endif
