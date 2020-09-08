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
  /* return entity->tags.size() > 0 && entity->tags.contains(tag_name); */
  return entity->tags.size() > 0 && entity->tags.count(tag_name) > 0;
}

void entity_get_all_with_name(
  Array<Entity> entities, const char *name,
  Array<Entity*> *found_entities
) {
  found_entities->size = 0;
  for (uint32 idx = 0; idx < entities.size; idx++) {
    Entity *entity = entities.items + idx;
    if (strcmp(entity->name, name) == 0) {
      assert(found_entities->size < found_entities->max_size);
      found_entities->items[found_entities->size++] = entity;
    }
  }
}

void entity_get_all_with_tag(
  Array<Entity> entities, const char *tag_name,
  Array<Entity*> *found_entities
) {
  found_entities->size = 0;
  for (uint32 idx = 0; idx < entities.size; idx++) {
    Entity *entity = entities.items + idx;
    if (entity_has_tag(entity, tag_name)) {
      assert(found_entities->size < found_entities->max_size);
      found_entities->items[found_entities->size++] = entity;
    }
  }
}
