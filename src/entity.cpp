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
