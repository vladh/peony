EntityManager::EntityManager(
  Array<Entity> *entities,
  Array<DrawableComponent> *drawable_components,
  Array<LightComponent> *light_components,
  Array<SpatialComponent> *spatial_components
) {
  this->entities = entities;
  this->drawable_components = drawable_components;
  this->light_components = light_components;
  this->spatial_components = spatial_components;
}


Entity* EntityManager::add(const char* debug_name) {
  Entity *new_entity = array_push<Entity>(entities);
  new_entity->handle = make_handle();
  new_entity->debug_name = debug_name;
  entity_handle_map[new_entity->handle] = new_entity;
  return new_entity;
}


void EntityManager::add_drawable_component(
  Entity *entity,
  ModelAsset *model_asset,
  RenderPass target_render_pass
) {
  entity->drawable = array_push(drawable_components);
  entity->drawable->model_asset = model_asset;
  entity->drawable->target_render_pass = target_render_pass;
}


void EntityManager::add_light_component(
  Entity *entity,
  glm::vec4 color,
  glm::vec4 attenuation
) {
  entity->light = array_push<LightComponent>(light_components);
  entity->light->color = color;
  entity->light->attenuation = attenuation;
}


void EntityManager::add_spatial_component(
  Entity *entity,
  glm::vec3 position,
  glm::quat rotation,
  glm::vec3 scale
) {
  entity->spatial = array_push<SpatialComponent>(spatial_components);
  entity->spatial->position = position;
  entity->spatial->rotation = rotation;
  entity->spatial->scale = scale;
}


Entity* EntityManager::get(EntityHandle handle) {
  if (entity_handle_map.count(handle)) {
    return entity_handle_map[handle];
  } else {
    return nullptr;
  }
}


void EntityManager::draw_all(
  RenderPass render_pass, RenderMode render_mode,
  ShaderAsset *entity_depth_shader_asset, uint32 shadow_light_idx
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
        &model_matrix, entity_depth_shader_asset,
        shadow_light_idx
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


EntityHandle EntityManager::make_handle() {
  return last_handle++;
}
