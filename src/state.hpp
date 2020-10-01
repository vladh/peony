#ifndef STATE_H
#define STATE_H

class State {
public:
  Camera camera_main;
  Camera *camera_active;
  Control control;

  real64 target_fps;
  real64 target_frame_duration_s;
  real64 t;
  real64 dt;
  real64 last_fps;
  real64 last_effective_fps;

  bool32 is_cursor_disabled;
  bool32 should_limit_fps;
  glm::vec4 background_color;

  uint32 window_width;
  uint32 window_height;
  char window_title[128];

  EntityManager entity_manager;
  DrawableComponentManager drawable_component_manager;
  LightComponentManager light_component_manager;
  SpatialComponentManager spatial_component_manager;

  Array<Entity> entities;
  Array<DrawableComponent> drawable_components;
  Array<LightComponent> light_components;
  Array<SpatialComponent> spatial_components;

  Array<EntityHandle> lights;
  Array<EntityHandle> geese;
  Array<EntityHandle> spheres;

  Array<ShaderAsset> shader_assets;
  Array<ModelAsset> model_assets;
  Array<FontAsset> font_assets;

  ShaderAsset *text_shader_asset;
  ShaderAsset *entity_depth_shader_asset;

  uint32 text_vao;
  uint32 text_vbo;

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
  uint32 g_position_texture;
  uint32 g_normal_texture;
  uint32 g_albedo_texture;
  uint32 g_pbr_texture;

  State(Memory *memory);
};

#endif
