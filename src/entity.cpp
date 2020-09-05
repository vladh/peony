#include "entity.hpp"


Entity* entity_make(
  Entity *entity, const char *name,
  EntityType type,
  glm::vec3 position, glm::vec3 scale, glm::quat rotation
) {
  // NOTE: Do we want all this C++ stuff here? Maybe not.
  entity = new(entity) Entity();
  entity->name = name;
  entity->type = type;
  entity->position = position;
  entity->scale = scale;
  entity->rotation = rotation;
  return entity;
}

void entity_set_position(Entity *entity, glm::vec3 position) {
  entity->position = position;
}

void entity_set_scale(Entity *entity, glm::vec3 scale) {
  entity->scale = scale;
}

void entity_set_rotation(Entity *entity, glm::quat rotation) {
  entity->rotation = rotation;
}

void entity_set_shader_asset(Entity *entity, ShaderAsset *asset) {
  entity->shader_asset = asset;
}

void entity_set_model_asset(Entity *entity, ModelAsset *asset) {
  entity->model_asset = asset;
}

void entity_add_tag(Entity *entity, const char *tag_name) {
  entity->tags.insert(EntityTag(tag_name));
}

void entity_remove_tag(Entity *entity, const char *tag_name) {
  entity->tags.erase(entity->tags.find(tag_name));
}

bool32 entity_has_tag(Entity *entity, const char *tag_name) {
  return entity->tags.size() > 0 && entity->tags.contains(tag_name);
}

Entity* entity_get_by_name(
  Entity *entities, uint32 n_entities, const char *name
) {
  for (uint32 idx = 0; idx < n_entities; idx++) {
    Entity *entity = entities + idx;
    if (strcmp(entity->name, name) == 0) {
      return entity;
    }
  }
  return nullptr;
}

void entity_get_all_with_tag(
  Entity *entities, uint32 n_entities, const char *tag_name,
  Entity *found_entities[], uint32 *n_found_entities
) {
  (*n_found_entities) = 0;
  for (uint32 idx = 0; idx < n_entities; idx++) {
    Entity *entity = entities + idx;
    if (entity_has_tag(entity, tag_name)) {
      found_entities[(*n_found_entities)++] = entity;
    }
  }
}
