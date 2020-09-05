#ifndef ENTITY_H
#define ENTITY_H
#pragma warning(disable : 4100)
#pragma warning(disable : 4201)

#include <set>

#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>

#include "shader.hpp"
#include "models.hpp"
#include "log.hpp"
#include "types.hpp"
#include "array.hpp"


enum EntityType {
  ENTITY_MODEL
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
void entity_set_shader_asset(Entity *entity, ShaderAsset *asset);
void entity_set_model_asset(Entity *entity, ModelAsset *asset);
void entity_add_tag(Entity *entity, const char *tag);
void entity_remove_tag(Entity *entity, const char *tag);
bool32 entity_has_tag(Entity *entity, const char *tag);
Entity* entity_get_by_name(
  Entity *entities, uint32 n_entities, const char *name
);
void entity_get_all_with_tag(
  Array<Entity> entities, const char *tag_name,
  Array<Entity*> *found_entities
);

#endif

