DrawableComponentManager::DrawableComponentManager(
  Array<DrawableComponent> *new_components
) {
  this->components = new_components;
}


DrawableComponent* DrawableComponentManager::add(
  EntityHandle entity_handle,
  ModelAsset *model_asset,
  RenderPass::Flag target_render_pass
) {
  if (!model_asset) {
    log_fatal("Invalid model_asset when creating DrawableComponent.");
  }
  DrawableComponent *new_component = this->components->push();
  new_component->entity_handle = entity_handle;
  new_component->model_asset = model_asset;
  new_component->target_render_pass = target_render_pass;
  return new_component;
}


DrawableComponent* DrawableComponentManager::get(EntityHandle handle) {
  // NOTE: Normally we'd use a hash-map or something here, but
  // std::unordered_map is slow as heck. This nice ol' array is faster.
  // Let's look for something else if this starts showing up in the profiler.
  for (uint32 idx = 0; idx < this->components->size; idx++) {
    if (this->components->get(idx)->entity_handle == handle) {
      return this->components->get(idx);
    }
  }
  return nullptr;
}


void DrawableComponentManager::draw_all(
  Memory *memory,
  PersistentPbo *persistent_pbo,
  TextureNamePool *texture_name_pool,
  SpatialComponentManager *spatial_component_manager,
  Queue<Task> *task_queue,
  RenderPass::Flag render_pass, RenderMode render_mode,
  ShaderAsset *standard_depth_shader_asset
) {
  for (uint32 idx = 0; idx < this->components->size; idx++) {
    DrawableComponent *drawable = this->components->get(idx);
    SpatialComponent *spatial = spatial_component_manager->get(drawable->entity_handle);

    if (!(render_pass & drawable->target_render_pass)) {
      continue;
    }

    glm::mat4 model_matrix = glm::mat4(1.0f);
    glm::mat3 model_normal_matrix = glm::mat4(1.0f);

    if (spatial) {
      model_matrix = spatial->make_model_matrix();
      model_normal_matrix = glm::mat3(glm::transpose(glm::inverse(model_matrix)));
    }

    if (render_mode == RenderMode::depth) {
      drawable->model_asset->draw_in_depth_mode(
        memory,
        persistent_pbo,
        texture_name_pool,
        task_queue,
        &model_matrix,
        &model_normal_matrix,
        standard_depth_shader_asset
      );
    } else {
      drawable->model_asset->draw(
        memory,
        persistent_pbo,
        texture_name_pool,
        task_queue,
        spatial ? &model_matrix : nullptr,
        spatial ? &model_normal_matrix : nullptr
      );
    }
  }
}
