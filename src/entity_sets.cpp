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


uint32 EntitySets::get_anim_channel_position_index_for_t(
  AnimChannel *anim_channel, real64 t
) {
  uint32 idx = anim_channel->last_position_key;
  do {
    if (
      t > anim_channel->position_keys[idx].time &&
      t < anim_channel->position_keys[idx + 1].time
    ) {
      anim_channel->last_position_key = idx;
      return idx;
    }
    idx++;
    if (idx == anim_channel->n_position_keys - 2) {
      idx = 0;
    }
  } while (idx != anim_channel->last_position_key);
  return 0;
}


uint32 EntitySets::get_anim_channel_rotation_index_for_t(
  AnimChannel *anim_channel, real64 t
) {
  uint32 idx = anim_channel->last_rotation_key;
  do {
    if (
      t > anim_channel->rotation_keys[idx].time &&
      t < anim_channel->rotation_keys[idx + 1].time
    ) {
      anim_channel->last_rotation_key = idx;
      return idx;
    }
    idx++;
    if (idx == anim_channel->n_rotation_keys - 2) {
      idx = 0;
    }
  } while (idx != anim_channel->last_rotation_key);
  return 0;
}


uint32 EntitySets::get_anim_channel_scaling_index_for_t(
  AnimChannel *anim_channel, real64 t
) {
  uint32 idx = anim_channel->last_scaling_key;
  do {
    if (
      t > anim_channel->scaling_keys[idx].time &&
      t < anim_channel->scaling_keys[idx + 1].time
    ) {
      anim_channel->last_scaling_key = idx;
      return idx;
    }
    idx++;
    if (idx == anim_channel->n_scaling_keys - 2) {
      idx = 0;
    }
  } while (idx != anim_channel->last_scaling_key);
  return 0;
}


void EntitySets::make_bone_matrices(
  glm::mat4 *local_bone_matrices,
  glm::mat4 *final_bone_matrices,
  AnimationComponent *animation_component,
  real64 t
) {
  if (animation_component->n_animations == 0) {
    return;
  }

  Animation *animation = &animation_component->animations[0];
  real64 duration_in_sec = animation->ticks_per_second * animation->duration;
  real64 animation_timepoint = fmod(t, duration_in_sec);

  for_range (0, animation_component->n_bones) {
    // NOTE: Because bones are sorted such that parents always come first,
    // it is guaranteed that each node's parent's matrix will always be set
    // when we get to computing the child.
    Bone *bone = &animation_component->bones[idx];
    glm::mat4 parent_transform = glm::mat4(1.0f);
    glm::mat4 translation = glm::mat4(1.0f);
    glm::mat4 rotation = glm::mat4(1.0f);
    glm::mat4 scale = glm::mat4(1.0f);

    if (idx > 0) {
      parent_transform = local_bone_matrices[bone->idx_parent];
    }

    AnimChannel *anim_channel = &animation->anim_channels[idx];

    if (anim_channel->n_position_keys > 0) {
      uint32 idx_position = get_anim_channel_position_index_for_t(
        anim_channel, animation_timepoint
      );
      translation = glm::translate(
        glm::mat4(1.0f), anim_channel->position_keys[idx_position].position
      );
    }

    if (anim_channel->n_rotation_keys > 0) {
      uint32 idx_rotation = get_anim_channel_rotation_index_for_t(
        anim_channel, animation_timepoint
      );
      rotation = glm::toMat4(
        anim_channel->rotation_keys[idx_rotation].rotation
      );
    }

    if (anim_channel->n_scaling_keys > 0) {
      uint32 idx_scaling = get_anim_channel_scaling_index_for_t(
        anim_channel, animation_timepoint
      );
      scale = glm::scale(
        glm::mat4(1.0f), anim_channel->scaling_keys[idx_scaling].scale
      );
    }

    glm::mat4 anim_transform = translation * rotation * scale;
    local_bone_matrices[idx] = parent_transform * anim_transform;
    final_bone_matrices[idx] =
      animation_component->scene_root_transform *
      local_bone_matrices[idx] *
      bone->offset *
      glm::inverse(animation_component->scene_root_transform);
  }
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
  bool32 have_animations,
  glm::mat4 *bone_matrices,
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
    if (Str::eq(uniform_name, "model_matrix")) {
      Shaders::set_mat4(shader_asset, "model_matrix", model_matrix);
    } else if (Str::eq(uniform_name, "model_normal_matrix")) {
      Shaders::set_mat3(shader_asset, "model_normal_matrix", model_normal_matrix);
    } else if (have_animations && Str::eq(uniform_name, "bone_matrices[0]")) {
      Shaders::set_mat4_multiple(
        shader_asset, MAX_N_BONES, "bone_matrices[0]", bone_matrices
      );
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
  EntitySet *entity_set,
  DrawableComponentSet *drawable_component_set,
  SpatialComponentSet *spatial_component_set,
  AnimationComponentSet *animation_component_set,
  Array<Material> *materials,
  RenderPassFlag render_pass,
  RenderMode render_mode,
  ShaderAsset *standard_depth_shader_asset,
  real64 t
) {
  ModelMatrixCache cache = {glm::mat4(1.0f), nullptr};

  for (uint32 idx = 0; idx < drawable_component_set->components.size; idx++) {
    /* Entity *entity = entity_set->entities.get(idx); */
    /* log_info("Drawing %s", entity->debug_name); */

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
    bool32 have_animations = false;
    glm::mat4 local_bone_matrices[MAX_N_BONES];
    glm::mat4 final_bone_matrices[MAX_N_BONES];

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
        have_animations = true;

        // TODO: This function is a bit heavy, and it needs a lot of space
        // allocated for all the matrices. Can we do this differently?
        // Should we preallocate some space for these matrices on the heap?
        // We should definitely remove the matrix inversion inside the
        // function. What else can we do to make it more lightweight?
        EntitySets::make_bone_matrices(
          local_bone_matrices,
          final_bone_matrices,
          animation_component,
          t
        );
      }
    }

    draw(
      render_mode,
      drawable_component_set,
      &drawable->mesh,
      material,
      &model_matrix,
      &model_normal_matrix,
      have_animations,
      final_bone_matrices,
      standard_depth_shader_asset
    );
  }
}
