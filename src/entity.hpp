#ifndef ENTITY_H
#define ENTITY_H

enum EntityType {
  ENTITY_MODEL, ENTITY_SCREENQUAD
};

struct EntityTag {
  const char *name;

  EntityTag(const char *name = "") : name(name) {
  }

  bool operator<(const EntityTag &rhs) const {
    return (bool)(strcmp(name, rhs.name) < 0);
  }

  bool operator==(const EntityTag &rhs) const {
    return (bool)(strcmp(name, rhs.name) == 0);
  }
};

struct Entity {
  RenderPass target_render_pass;
  const char *name;
  EntityType type;
  glm::vec3 position;
  glm::vec3 scale;
  glm::quat rotation;
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

