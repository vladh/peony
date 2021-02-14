#ifndef STATE_HPP
#define STATE_HPP

constexpr uint32 DT_HIST_LENGTH = 512;

struct WindowInfo {
  GLFWwindow *window;
  uint32 width;
  uint32 height;
  char title[128];
};

class State {
public:
  bool32 is_manual_frame_advance_enabled;
  bool32 should_manually_advance_to_next_frame;
  bool32 should_stop;
  bool32 should_pause;
  bool32 should_hide_ui;
  WindowInfo window_info;

  Cameras::Camera camera_main;
  Cameras::Camera *camera_active;
  Input::InputState input_state;

  real64 t;
  real64 dt;
  real64 dt_average;
  real64 dt_hist[DT_HIST_LENGTH];
  uint32 dt_hist_idx;
  uint32 last_fps;
  uint32 n_frames_since_start;

  bool32 is_cursor_disabled;
  bool32 should_limit_fps;
  bool32 should_use_wireframe;
  glm::vec4 background_color;

  Array<Shaders::ShaderAsset> shader_assets;
  Array<Models::ModelAsset> model_assets;

  Array<Entity> entities;
  Array<DrawableComponent> drawable_components;
  Array<LightComponent> light_components;
  Array<SpatialComponent> spatial_components;
  Array<BehaviorComponent> behavior_components;
  Gui::GuiState gui_state;

  EntityManager entity_manager;
  DrawableComponentManager drawable_component_manager;
  LightComponentManager light_component_manager;
  SpatialComponentManager spatial_component_manager;
  BehaviorComponentManager behavior_component_manager;

  real32 heading_opacity;
  const char *heading_text;
  real32 heading_fadeout_duration;
  real32 heading_fadeout_delay;

  Textures::PersistentPbo persistent_pbo;
  Textures::TextureNamePool texture_name_pool;
  Queue<Tasks::Task> task_queue;

  uint32 cube_shadowmap_width;
  uint32 cube_shadowmap_height;
  uint32 texture_shadowmap_width;
  uint32 texture_shadowmap_height;
  real32 shadowmap_near_clip_dist;
  real32 shadowmap_far_clip_dist;

  real32 dir_light_angle;

  uint32 cube_shadowmaps_framebuffer;
  uint32 cube_shadowmaps;
  uint32 texture_shadowmaps_framebuffer;
  uint32 texture_shadowmaps;
  glm::mat4 cube_shadowmap_transforms[6 * MAX_N_LIGHTS];
  glm::mat4 texture_shadowmap_transforms[MAX_N_LIGHTS];
  Textures::Texture *cube_shadowmaps_texture;
  Textures::Texture *texture_shadowmaps_texture;

  Shaders::ShaderAsset *standard_depth_shader_asset;

  uint32 ubo_shader_common;
  Renderer::ShaderCommon shader_common;

  uint32 g_buffer;
  Textures::Texture *g_position_texture;
  Textures::Texture *g_normal_texture;
  Textures::Texture *g_albedo_texture;
  Textures::Texture *g_pbr_texture;

  uint32 l_buffer;
  Textures::Texture *l_color_texture;
  Textures::Texture *l_bright_color_texture;
  Textures::Texture *l_depth_texture;

  uint32 blur1_buffer;
  uint32 blur2_buffer;
  Textures::Texture *blur1_texture;
  Textures::Texture *blur2_texture;

  State(Memory *memory, WindowInfo window_info);
};

struct MemoryAndState {
  Memory *memory;
  State *state;
};

#endif
