#ifndef ENTITY_H
#define ENTITY_H

typedef uint32 EntityHandle;

enum EntityBehaviour {
  BEHAVIOUR_SHOWCASE_LIGHT,
  BEHAVIOUR_SPINNY_BOI
};

struct Component {
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

class Entity {
public:
  EntityHandle handle;
  const char *debug_name;
  DrawableComponent *drawable;
  LightComponent *light;
  SpatialComponent *spatial;

  void print();
};

#endif
