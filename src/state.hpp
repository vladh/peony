#ifndef STATE_H
#define STATE_H

struct WindowInfo {
  GLFWwindow *window;
  uint32 width;
  uint32 height;
  char title[128];
};

class State {
public:
  bool32 should_stop;
  bool32 should_pause;
  bool32 should_hide_ui;
  WindowInfo window_info;

  Camera camera_main;
  Camera *camera_active;
  Control control;

  real64 t;
  real64 dt;
  real64 last_fps;

  bool32 is_cursor_disabled;
  bool32 should_limit_fps;
  bool32 should_use_wireframe;
  glm::vec4 background_color;

  Array<ShaderAsset> shader_assets;
  Array<ModelAsset> model_assets;

  Array<Entity> entities;
  Array<DrawableComponent> drawable_components;
  Array<LightComponent> light_components;
  Array<SpatialComponent> spatial_components;

  EntityManager entity_manager;
  DrawableComponentManager drawable_component_manager;
  LightComponentManager light_component_manager;
  SpatialComponentManager spatial_component_manager;
  TextManager text_manager;

  Array<EntityHandle> lights;
  Array<EntityHandle> geese;
  Array<EntityHandle> spheres;

  PersistentPbo persistent_pbo;
  TextureNamePool texture_name_pool;
  Queue<Task> task_queue;

  ShaderAsset *standard_depth_shader_asset;

  uint32 shadow_map_width;
  uint32 shadow_map_height;
  real32 shadow_near_clip_dist;
  real32 shadow_far_clip_dist;
  uint32 shadow_framebuffers[MAX_N_SHADOW_FRAMEBUFFERS];
  uint32 shadow_cubemaps[MAX_N_SHADOW_FRAMEBUFFERS];
  uint32 n_shadow_framebuffers;
  uint32 shadow_light_idx;
  glm::mat4 shadow_transforms[6];

  uint32 ubo_shader_common;
  ShaderCommon shader_common;

  uint32 g_buffer;
  Texture *g_position_texture;
  Texture *g_normal_texture;
  Texture *g_albedo_texture;
  Texture *g_pbr_texture;

  State(Memory *memory, WindowInfo window_info);
};

#endif
