#ifndef ENTITY_H
#define ENTITY_H

#include <set>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "shader.hpp"
#include "models.hpp"
#include "log.hpp"
#include "types.hpp"
#include "array.hpp"
enum EntityType { ENTITY_MODEL
};

struct EntityTag {
  const char *name;

  EntityTag(const char *name = "") : name(name) {
  }

  bool operator<(const EntityTag &rhs) const {
    return strcmp(name, rhs.name);
  }

  bool operator==(const EntityTag &rhs) const {
    return strcmp(name, rhs.name);
  }
};

struct Entity {
  const char *name;
  EntityType type;
  glm::vec3 position;
  glm::vec3 scale;
  glm::quat rotation;
  glm::vec3 color;
  ShaderAsset *shader_asset;
  ModelAsset *model_asset;
  std::set<EntityTag> tags;
};

Entity* entity_make(
  Entity *entity, const char *name,
  EntityType type,
  glm::vec3 position, glm::vec3 scale, glm::quat rotation
);
void entity_set_position(Entity *entity, glm::vec3 position);
void entity_set_scale(Entity *entity, glm::vec3 scale);
void entity_set_rotation(Entity *entity, glm::quat rotation);
void entity_set_color(Entity *entity, glm::vec3 color);
void entity_set_shader_asset(Entity *entity, ShaderAsset *asset);
void entity_set_model_asset(Entity *entity, ModelAsset *asset);
void entity_add_tag(Entity *entity, const char *tag);
void entity_remove_tag(Entity *entity, const char *tag);
bool32 entity_has_tag(Entity *entity, const char *tag);
void entity_get_all_with_name(
  Array<Entity> entities, const char *name,
  Array<Entity*> *found_entities
);
void entity_get_all_with_tag(
  Array<Entity> entities, const char *tag_name,
  Array<Entity*> *found_entities
);

#endif

