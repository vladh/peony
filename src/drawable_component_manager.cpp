DrawableComponentManager::DrawableComponentManager(
  Array<DrawableComponent> *components
) {
  this->components = components;
}


DrawableComponent* DrawableComponentManager::add(
  EntityHandle entity_handle,
  ModelAsset *model_asset,
  RenderPass target_render_pass
) {
  DrawableComponent *new_component = array_push<DrawableComponent>(components);
  new_component->entity_handle = entity_handle;
  new_component->model_asset = model_asset;
  new_component->target_render_pass = target_render_pass;
  return new_component;
}


DrawableComponent* DrawableComponentManager::get(EntityHandle handle) {
  // NOTE: Normally we'd use a hash-map or something here, but
  // std::unordered_map is slow as heck. This nice ol' array is faster.
  // Let's look for something else if this starts showing up in the profiler.
  for (uint32 idx = 0; idx < components->size; idx++) {
    if (components->items[idx].entity_handle == handle) {
      return &components->items[idx];
    }
  }
  return nullptr;
}


void DrawableComponentManager::draw_all(
  SpatialComponentManager spatial_component_manager,
  RenderPass render_pass, RenderMode render_mode,
  ShaderAsset *entity_depth_shader_asset
) {
  for (uint32 idx = 0; idx < components->size; idx++) {
    DrawableComponent *drawable = &components->items[idx];
    SpatialComponent *spatial = spatial_component_manager.get(drawable->entity_handle);

    if (render_pass != drawable->target_render_pass) {
      continue;
    }

    glm::mat4 model_matrix = glm::mat4(1.0f);

    if (spatial) {
      // TODO: This is somehow really #slow, the multiplication in particular.
      // Is there a better way?
      model_matrix = glm::translate(model_matrix, spatial->position);
      model_matrix = glm::scale(model_matrix, spatial->scale);
      model_matrix = model_matrix * glm::toMat4(spatial->rotation);
    }

    if (render_mode == RENDERMODE_DEPTH) {
      models_draw_model_in_depth_mode(
        drawable->model_asset,
        &model_matrix, entity_depth_shader_asset
      );
    } else {
      models_draw_model(
        drawable->model_asset,
        spatial ? &model_matrix : nullptr
      );
    }
  }
}