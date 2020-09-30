#ifndef COMPONENT_H
#define COMPONENT_H

struct Component {
  EntityHandle entity_handle;
};

struct DrawableComponent : Component {
  ModelAsset *model_asset;
  RenderPass target_render_pass;
};

struct LightComponent : Component {
  glm::vec4 color;
  glm::vec4 attenuation;
};

struct SpatialComponent : Component {
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 scale;
};

#endif
