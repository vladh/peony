Entity* entity_make(
  Entity *entity, const char *name,
  EntityType type, RenderPass target_render_pass,
  glm::vec3 position, glm::vec3 scale, glm::quat rotation
) {
  // NOTE: Do we want all this C++ stuff here? Maybe not.
  entity = new(entity) Entity();
  entity->name = name;
  entity->type = type;
  entity->target_render_pass = target_render_pass;
  entity->position = position;
  entity->scale = scale;
  entity->rotation = rotation;
  entity->model_asset = nullptr;
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

void entity_set_model_asset(Entity *entity, ModelAsset *asset) {
  entity->model_asset = asset;
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
