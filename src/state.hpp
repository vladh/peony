#ifndef STATE_H
#define STATE_H

struct Light {
  bool is_point_light;
  glm::vec3 position;
  glm::vec3 direction;
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  real32 attenuation_constant;
  real32 attenuation_linear;
  real32 attenuation_quadratic;
};

struct State {
  Camera camera;
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
  Array<ShaderAsset> shader_assets;
  Array<ModelAsset*> model_assets;
  Array<Light> lights;

  uint32 postprocessing_framebuffer;

  uint32 shadow_map_width;
  uint32 shadow_map_height;
  uint32 shadow_fbo;
};

#endif
