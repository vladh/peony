EntityHandle EntitySets::make_handle(
  EntitySet *entity_set
) {
  // NOTE: 0 is an invalid handle.
  if (entity_set->next_handle == 0) {
    entity_set->next_handle++;
  }
  return entity_set->next_handle++;
}


Entity* EntitySets::add_entity_to_set(
  EntitySet *entity_set,
  const char *debug_name
) {
  EntityHandle new_handle = make_handle(entity_set);
  Entity *new_entity = entity_set->entities.get(new_handle);
  new_entity->handle = new_handle;
  strcpy(new_entity->debug_name, debug_name);
  return new_entity;
}


glm::mat4 EntitySets::make_model_matrix(
  SpatialComponentSet *spatial_component_set,
  SpatialComponent *spatial_component,
  ModelMatrixCache *cache
) {
  glm::mat4 model_matrix = glm::mat4(1.0f);

  if (spatial_component->parent_entity_handle != Entity::no_entity_handle) {
    SpatialComponent *parent = spatial_component_set->components.get(
      spatial_component->parent_entity_handle
    );
    model_matrix = make_model_matrix(spatial_component_set, parent, cache);
  }

  if (Entities::does_spatial_component_have_dimensions(spatial_component)) {
    // TODO: This is somehow really #slow, the multiplication in particular.
    // Is there a better way?
    if (
      spatial_component == cache->last_model_matrix_spatial_component
    ) {
      model_matrix = cache->last_model_matrix;
    } else {
      model_matrix = glm::translate(model_matrix, spatial_component->position);
      model_matrix = glm::scale(model_matrix, spatial_component->scale);
      model_matrix = model_matrix * glm::toMat4(spatial_component->rotation);
      cache->last_model_matrix = model_matrix;
      cache->last_model_matrix_spatial_component = spatial_component;
    }
  }

  return model_matrix;
}


void EntitySets::make_bone_matrices(
  glm::mat4 *bones_matrices,
  AnimationComponent *animation_component
) {
  log_info("Making bone matrices for entity %d", animation_component->entity_handle);
}


AnimationComponent* EntitySets::find_animation_component(
  SpatialComponent *spatial_component,
  SpatialComponentSet *spatial_component_set,
  AnimationComponentSet *animation_component_set
) {
  AnimationComponent *animation_component = animation_component_set->components.get(
    spatial_component->entity_handle
  );

  if (Entities::is_animation_component_valid(animation_component)) {
    return animation_component;
  }

  if (spatial_component->parent_entity_handle != Entity::no_entity_handle) {
    SpatialComponent *parent = spatial_component_set->components.get(
      spatial_component->parent_entity_handle
    );
    return find_animation_component(
      parent,
      spatial_component_set,
      animation_component_set
    );
  }

  return nullptr;
}


void EntitySets::draw(
  RenderMode render_mode,
  DrawableComponentSet *drawable_component_set,
  Mesh *mesh,
  Material *material,
  glm::mat4 *model_matrix,
  glm::mat3 *model_normal_matrix,
  ShaderAsset *standard_depth_shader_asset
) {
  ShaderAsset *shader_asset = nullptr;

  if (render_mode == RenderMode::regular) {
    shader_asset = &material->shader_asset;
  } else if (render_mode == RenderMode::depth) {
    if (Shaders::is_shader_asset_valid(&material->depth_shader_asset)) {
      shader_asset = &material->depth_shader_asset;
    } else {
      shader_asset = standard_depth_shader_asset;
    }
  }

  assert(shader_asset);

  // If our shader program has changed since our last mesh, tell OpenGL about it.
  if (shader_asset->program != drawable_component_set->last_drawn_shader_program) {
    glUseProgram(shader_asset->program);
    drawable_component_set->last_drawn_shader_program = shader_asset->program;

    if (render_mode == RenderMode::regular) {
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
    }
  }

  for (
    uint32 uniform_idx = 0;
    uniform_idx < shader_asset->n_intrinsic_uniforms;
    uniform_idx++
  ) {
    const char *uniform_name = shader_asset->intrinsic_uniform_names[uniform_idx];
    if (strcmp(uniform_name, "model_matrix") == 0) {
      Shaders::set_mat4(shader_asset, "model_matrix", model_matrix);
    } else if (strcmp(uniform_name, "model_normal_matrix") == 0) {
      Shaders::set_mat3(shader_asset, "model_normal_matrix", model_normal_matrix);
    }
  }

  glBindVertexArray(mesh->vao);
  if (mesh->n_indices > 0) {
    glDrawElements(mesh->mode, mesh->n_indices, GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(mesh->mode, 0, mesh->n_vertices);
  }
}


void EntitySets::draw_all(
  DrawableComponentSet *drawable_component_set,
  SpatialComponentSet *spatial_component_set,
  AnimationComponentSet *animation_component_set,
  Array<Material> *materials,
  RenderPassFlag render_pass,
  RenderMode render_mode,
  ShaderAsset *standard_depth_shader_asset
) {
  ModelMatrixCache cache = {glm::mat4(1.0f), nullptr};

  for (uint32 idx = 0; idx < drawable_component_set->components.size; idx++) {
    DrawableComponent *drawable = drawable_component_set->components.get(idx);

    if (!Models::is_drawable_component_valid(drawable)) {
      continue;
    }

    if (!(render_pass & drawable->target_render_pass)) {
      continue;
    }

    Material *material = Materials::get_material_by_name(
      materials, drawable->mesh.material_name
    );

    if (!material || material->state != MaterialState::complete) {
      material = Materials::get_material_by_name(materials, "unknown");
    }

    SpatialComponent *spatial = spatial_component_set->components.get(
      drawable->entity_handle
    );

    glm::mat4 model_matrix = glm::mat4(1.0f);
    glm::mat3 model_normal_matrix = glm::mat3(1.0f);
    glm::mat4 bone_matrices[MAX_N_BONES];

    if (Entities::is_spatial_component_valid(spatial)) {
      // We only need to calculate the normal matrix if we have non-uniform
      // scaling.
      model_matrix = make_model_matrix(spatial_component_set, spatial, &cache);

      // TODO: Even though we have a uniform scaling in the transformation for
      // our spatial component itself, when accumulating it with the parent
      // spatial components, we might (possibly) get non-uniform scaling,
      // in which case we want to calculate the model normal matrix!
      // Oops! We should be looking at the model_matrix and not at spatial->scale.
      if (
        spatial->scale.x == spatial->scale.y &&
        spatial->scale.y == spatial->scale.z
      ) {
        model_normal_matrix = glm::mat3(model_matrix);
      } else {
        model_normal_matrix = glm::mat3(glm::transpose(glm::inverse(model_matrix)));
      }

      // Animations
      AnimationComponent *animation_component = find_animation_component(
        spatial,
        spatial_component_set,
        animation_component_set
      );
      if (animation_component) {
        EntitySets::make_bone_matrices(bone_matrices, animation_component);
      }
    }

    draw(
      render_mode,
      drawable_component_set,
      &drawable->mesh,
      material,
      &model_matrix,
      &model_normal_matrix,
      standard_depth_shader_asset
    );
  }
}
