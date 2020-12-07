uint32 DrawableComponentManager::last_drawn_shader_program = 0;

DrawableComponentManager::DrawableComponentManager(
  Array<DrawableComponent> *new_components
) {
  this->components = new_components;
}


DrawableComponent* DrawableComponentManager::add(
  EntityHandle entity_handle,
  Mesh *mesh,
  RenderPass::Flag target_render_pass
) {
  assert(entity_handle > 0);
  if (!mesh) {
    log_fatal("Invalid mesh when creating DrawableComponent.");
  }
  DrawableComponent *new_component = this->components->get(entity_handle);
  new_component->entity_handle = entity_handle;
  new_component->mesh = mesh;
  new_component->target_render_pass = target_render_pass;
  return new_component;
}


DrawableComponent* DrawableComponentManager::get(EntityHandle handle) {
  assert(handle > 0);
  return this->components->get(handle);
}


void DrawableComponentManager::draw_all(
  SpatialComponentManager *spatial_component_manager,
  RenderPass::Flag render_pass, RenderMode render_mode,
  ShaderAsset *standard_depth_shader_asset
) {
  for (uint32 idx = 0; idx < this->components->size; idx++) {
    DrawableComponent *drawable = this->components->get(idx);

    if (!drawable->mesh) {
      continue;
    }

    SpatialComponent *spatial = spatial_component_manager->get(drawable->entity_handle);

    if (!(render_pass & drawable->target_render_pass)) {
      continue;
    }

    glm::mat4 model_matrix = glm::mat4(1.0f);
    glm::mat3 model_normal_matrix = glm::mat3(1.0f);

    if (spatial) {
      // We only need to calculate the normal matrix if we have non-uniform
      // scaling.
      model_matrix = spatial_component_manager->make_model_matrix(spatial);
      if (
        spatial->scale.x == spatial->scale.y &&
        spatial->scale.y == spatial->scale.z
      ) {
        model_normal_matrix = glm::mat3(model_matrix);
      } else {
        model_normal_matrix = glm::mat3(glm::transpose(glm::inverse(model_matrix)));
      }
    }

    if (render_mode == RenderMode::depth) {
      draw_in_depth_mode(
        drawable->mesh,
        &model_matrix,
        &model_normal_matrix,
        standard_depth_shader_asset
      );
    } else {
      draw(
        drawable->mesh,
        &model_matrix,
        &model_normal_matrix
      );
    }
  }
}


void DrawableComponentManager::draw(
  Mesh *mesh,
  glm::mat4 *model_matrix,
  glm::mat3 *model_normal_matrix
) {
  ShaderAsset *shader_asset = mesh->material->shader_asset;

  // If our shader program has changed since our last mesh, tell OpenGL about it.
  if (shader_asset->program != DrawableComponentManager::last_drawn_shader_program) {
    glUseProgram(shader_asset->program);
    DrawableComponentManager::last_drawn_shader_program = shader_asset->program;

    for (
      uint32 texture_idx = 1;
      texture_idx < shader_asset->n_texture_units + 1; texture_idx++
    ) {
      if (shader_asset->texture_units[texture_idx] != 0) {
        glActiveTexture(GL_TEXTURE0 + texture_idx);
        glBindTexture(
          shader_asset->texture_unit_types[texture_idx],
          shader_asset->texture_units[texture_idx]
        );
      }
    }

    for (
      uint32 uniform_idx = 0;
      uniform_idx < shader_asset->n_intrinsic_uniforms;
      uniform_idx++
    ) {
      const char *uniform_name = shader_asset->intrinsic_uniform_names[uniform_idx];
      if (strcmp(uniform_name, "model_matrix") == 0) {
        shader_asset->set_mat4("model_matrix", model_matrix);
      } else if (strcmp(uniform_name, "model_normal_matrix") == 0) {
        shader_asset->set_mat3("model_normal_matrix", model_normal_matrix);
      }
    }
  }

  glBindVertexArray(mesh->vao);
  if (mesh->n_indices > 0) {
    glDrawElements(mesh->mode, mesh->n_indices, GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(mesh->mode, 0, mesh->n_vertices);
  }
}


void DrawableComponentManager::draw_in_depth_mode(
  Mesh *mesh,
  glm::mat4 *model_matrix,
  glm::mat3 *model_normal_matrix,
  ShaderAsset *standard_depth_shader_asset
) {
  ShaderAsset *shader_asset = standard_depth_shader_asset;

  if (mesh->material->depth_shader_asset) {
    shader_asset = mesh->material->depth_shader_asset;
  }

  // If our shader program has changed since our last mesh, tell OpenGL about it.
  if (shader_asset->program != DrawableComponentManager::last_drawn_shader_program) {
    glUseProgram(shader_asset->program);
    DrawableComponentManager::last_drawn_shader_program = shader_asset->program;

    for (
      uint32 uniform_idx = 0;
      uniform_idx < shader_asset->n_intrinsic_uniforms;
      uniform_idx++
    ) {
      const char *uniform_name = shader_asset->intrinsic_uniform_names[uniform_idx];
      if (strcmp(uniform_name, "model_matrix") == 0) {
        shader_asset->set_mat4("model_matrix", model_matrix);
      } else if (strcmp(uniform_name, "model_normal_matrix") == 0) {
        shader_asset->set_mat3("model_normal_matrix", model_normal_matrix);
      }
    }
  }

  glBindVertexArray(mesh->vao);
  if (mesh->n_indices > 0) {
    glDrawElements(mesh->mode, mesh->n_indices, GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(mesh->mode, 0, mesh->n_vertices);
  }
}
