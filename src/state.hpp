#ifndef STATE_H
#define STATE_H

struct State {
  Camera camera_main;
  Camera *camera_active;
  Control control;

  real64 t;
  real64 dt;
  bool32 is_wireframe_on;
  bool32 is_cursor_disabled;
  glm::vec4 background_color;

  uint32 window_width;
  uint32 window_height;
  char window_title[32];

  Array<Entity> entities;
  Array<Entity*> found_entities;
  ShaderAsset entity_shader_asset;
  ShaderAsset entity_depth_shader_asset;

  Array<ShaderAsset> shader_assets;
  Array<ModelAsset*> model_assets;
  Array<Light> lights;

  uint32 postprocessing_framebuffer;

  uint32 shadow_map_width;
  uint32 shadow_map_height;
  real32 shadow_near_clip_dist;
  real32 shadow_far_clip_dist;
  uint32 shadow_framebuffer;

  uint32 shadow_map_texture;

  uint32 shadow_cubemap;
  glm::mat4 shadow_transforms[6];

  RenderMode render_mode;
};

#endif
