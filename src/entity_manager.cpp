EntityManager::EntityManager(Array<Entity> *entities) {
  this->entities = entities;
}


Entity* EntityManager::add(const char* debug_name) {
  Entity *new_entity = array_push<Entity>(entities);
  new_entity->handle = make_handle();
  new_entity->debug_name = debug_name;
  entity_handle_map[new_entity->handle] = new_entity;
  return new_entity;
}


Entity* EntityManager::get(EntityHandle handle) {
  if (entity_handle_map.count(handle)) {
    return entity_handle_map[handle];
  } else {
    return nullptr;
  }
}


#if 0
void EntityManager::draw_all(
  RenderPass render_pass, RenderMode render_mode,
  ShaderAsset *entity_depth_shader_asset
) {
  for (uint32 idx = 0; idx < entities->size; idx++) {
    Entity *entity = &entities->items[idx];

    if (!entity->drawable) {
      log_warning("Trying to draw entity %s with no drawable component.", entity->debug_name);
      continue;
    }

    if (render_pass != entity->drawable->target_render_pass) {
      continue;
    }

    glm::mat4 model_matrix = glm::mat4(1.0f);

    if (entity->spatial) {
      // TODO: This is somehow really #slow, the multiplication in particular.
      // Is there a better way?
      model_matrix = glm::translate(model_matrix, entity->spatial->position);
      model_matrix = glm::scale(model_matrix, entity->spatial->scale);
      model_matrix = model_matrix * glm::toMat4(entity->spatial->rotation);
    }

    if (render_mode == RENDERMODE_DEPTH) {
      models_draw_model_in_depth_mode(
        entity->drawable->model_asset,
        &model_matrix, entity_depth_shader_asset
      );
    } else {
      models_draw_model(
        entity->drawable->model_asset,
        entity->spatial ? &model_matrix : nullptr
      );
    }
  }
}


void EntityManager::print_all() {
  for (uint32 idx = 0; idx < entities->size; idx++) {
    Entity *entity = &entities->items[idx];
    entity->print();
  }
}
#endif


EntityHandle EntityManager::make_handle() {
  return last_handle++;
}
