void Entity::draw(
  RenderPass render_pass, RenderMode render_mode,
  ShaderAsset *entity_depth_shader_asset
) {
  if (!drawable) {
    log_warning("Trying to draw entity %d with no drawable component.", handle);
    return;
  }

  if (render_pass != drawable->target_render_pass) {
    return;
  }

  if (spatial) {
    // TODO: This is somehow really #slow, the multiplication in particular.
    // Is there a better way?
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, spatial->position);
    model_matrix = glm::scale(model_matrix, spatial->scale);
    model_matrix = model_matrix * glm::toMat4(spatial->rotation);
    models_draw_model(
      drawable->model_asset, render_mode, &model_matrix, entity_depth_shader_asset
    );
  } else {
    models_draw_model(
      drawable->model_asset, render_mode, nullptr, entity_depth_shader_asset
    );
  }
}


void Entity::print() {
  log_info("# Entity %d: %s", handle, debug_name);
  if (drawable) {
    log_info("-> Drawable");
    log_info("   model_asset %x", drawable->model_asset);
    log_info("     info.name %s", drawable->model_asset->info.name);
    log_info("     model.meshes.size %d",
      drawable->model_asset->model.meshes.size
    );
    log_info("     model.texture_sets.size %d",
      drawable->model_asset->model.texture_sets.size
    );
    log_info("   target_render_pass %d", drawable->target_render_pass);
  }
  if (light) {
    log_info("-> Light");
    log_info("   color (%f,%f,%f)", light->color.r, light->color.g, light->color.b);
    log_info("   attenuation (%f,%f,%f,%f)",
      light->attenuation[0], light->attenuation[1],
      light->attenuation[2], light->attenuation[3]
    );
  }
  if (spatial) {
    log_info("-> Spatial");
    log_info("   position (%f,%f,%f)",
      spatial->position.x, spatial->position.y, spatial->position.z
    );
    log_info("   rotation (%f %f,%f,%f)",
      spatial->rotation.w, spatial->rotation.x, spatial->rotation.y, spatial->rotation.z
    );
    log_info("   scale (%f,%f,%f)",
      spatial->scale.x, spatial->scale.y, spatial->scale.z
    );
  }
  log_newline();
}
