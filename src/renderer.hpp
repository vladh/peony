#ifndef OPENGL_HPP
#define OPENGL_HPP

constexpr bool32 USE_OPENGL_DEBUG = false;
constexpr uint16 MAX_N_LIGHTS = 8;

// The position is used in positioning the shadow map, but not
// in the light calculations. We need to scale the position
// by some factor such that the shadow map covers the biggest
// possible area.
constexpr glm::vec3 DIRECTIONAL_LIGHT_DISTANCE = glm::vec3(
  75.0f, 15.0f, 75.0f
);

struct WindowInfo;

namespace Renderer {
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
  };

  struct ShaderCommon {
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 ui_projection;
    glm::mat4 cube_shadowmap_transforms[6 * MAX_N_LIGHTS];
    glm::mat4 texture_shadowmap_transforms[MAX_N_LIGHTS];

    glm::vec3 camera_position;
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
    float pad_1;
    float pad_2;

    float exposure;
    float t;
    int window_width;
    int window_height;

    glm::vec4 point_light_position[MAX_N_LIGHTS];
    glm::vec4 point_light_color[MAX_N_LIGHTS];
    glm::vec4 point_light_attenuation[MAX_N_LIGHTS];

    glm::vec4 directional_light_position[MAX_N_LIGHTS];
    glm::vec4 directional_light_direction[MAX_N_LIGHTS];
    glm::vec4 directional_light_color[MAX_N_LIGHTS];
    glm::vec4 directional_light_attenuation[MAX_N_LIGHTS];
  };

  const char* render_pass_to_string(RenderPassFlag render_pass);
  RenderPassFlag render_pass_from_string(const char* str);
  void resize_renderer_buffers(Memory *memory, State *state);
  void init_ubo(Memory *memory, State *state);
  void init_shadowmaps(Memory *memory, State *state);
  void init_g_buffer(Memory *memory, State *state);
  void init_l_buffer(Memory *memory, State *state);
  void init_blur_buffers(Memory *memory, State *state);
  void update_drawing_options(State *state, GLFWwindow *window);
  void copy_scene_data_to_ubo(
    Memory *memory, State *state,
    uint32 current_shadow_light_idx,
    uint32 current_shadow_light_type,
    bool32 is_blur_horizontal
  );
  void copy_scene_data_to_ubo(Memory *memory, State *state);
  void init_window(WindowInfo *window_info);
  void destroy_window();
  void framebuffer_size_callback(GLFWwindow* window, int width, int height);
  void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
  void mouse_callback(GLFWwindow *window, real64 x, real64 y);
  void key_callback(
    GLFWwindow* window, int key, int scancode, int action, int mods
  );
  void reload_shaders(Memory *memory, State *state);
  void render_scene(
    Memory *memory,
    State *state,
    Renderer::RenderPassFlag render_pass,
    Renderer::RenderMode render_mode
  );
  void set_heading(
    State *state,
    const char *text, real32 opacity,
    real32 fadeout_duration, real32 fadeout_delay
  );
  void render_scene_ui(
    Memory *memory, State *state
  );
  void render(Memory *memory, State *state);
}

#endif
