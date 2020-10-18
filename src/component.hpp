#ifndef COMPONENT_H
#define COMPONENT_H

struct Component {
  EntityHandle entity_handle;
};

struct DrawableComponent : public Component {
  ModelAsset *model_asset;
  RenderPass target_render_pass;
};

struct LightComponent : public Component {
  LightType type;
  glm::vec3 direction;
  glm::vec4 color;
  glm::vec4 attenuation;
};

struct SpatialComponent : public Component {
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 scale;
};

#endif
