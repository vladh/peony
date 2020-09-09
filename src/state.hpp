#ifndef STATE_H
#define STATE_H

#include <stdint.h>

#include <glm/glm.hpp>

#include "types.hpp"
#include "asset.hpp"
#include "models.hpp"
#include "control.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "entity.hpp"
#include "array.hpp"


struct Light {
  glm::vec3 position;
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

  uint32 window_width;
  uint32 window_height;
  char window_title[32];

  glm::vec3 light_position;

  Array<Entity> entities;
  Array<Entity*> found_entities;
  Array<ShaderAsset> shader_assets;
  Array<ModelAsset*> model_assets;
  Array<Light> lights;
};

#endif
