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
      model_matrix = model_matrix *
        glm::toMat4(glm::normalize(spatial_component->rotation));
      cache->last_model_matrix = model_matrix;
      cache->last_model_matrix_spatial_component = spatial_component;
    }
  }

  return model_matrix;
}


void EntitySets::update_animation_components(
  AnimationComponentSet *animation_component_set,
  SpatialComponentSet *spatial_component_set,
  real64 t
) {
  for (
    uint32 idx = 1;
    idx < animation_component_set->components.size;
    idx++
  ) {
    AnimationComponent *animation_component =
      animation_component_set->components.get(idx);

    if (!Entities::is_animation_component_valid(animation_component)) {
      continue;
    }

    make_bone_matrices(animation_component, t);
  }
}


void EntitySets::update_behavior_components(
  BehaviorComponentSet *behavior_component_set,
  SpatialComponentSet *spatial_component_set,
  real64 t
) {
  for (
    uint32 idx = 1;
    idx < behavior_component_set->components.size;
    idx++
  ) {
    BehaviorComponent *behavior_component =
      behavior_component_set->components.get(idx);

    if (
      !behavior_component ||
      !Entities::is_behavior_component_valid(behavior_component)
    ) {
      continue;
    }

    EntityHandle entity_handle = behavior_component->entity_handle;

    SpatialComponent *spatial_component =
      spatial_component_set->components.get(entity_handle);
    if (!spatial_component) {
      log_error("Could not get SpatialComponent for BehaviorComponent");
      continue;
    }

    if (behavior_component->behavior == Behavior::test) {
      spatial_component->position = glm::vec3(
        (real32)sin(t) * 15.0f,
        (real32)((sin(t * 2.0f) + 1.5) * 3.0f),
        (real32)cos(t) * 15.0f
      );
    }
  }
}


void EntitySets::update_light_components(
  LightComponentSet *light_component_set,
  SpatialComponentSet *spatial_component_set,
  real64 t,
  glm::vec3 camera_position
) {
  for (uint32 idx = 0; idx < light_component_set->components.size; idx++) {
    LightComponent *light_component =
      light_component_set->components.get(idx);
    SpatialComponent *spatial_component =
      spatial_component_set->components.get(light_component->entity_handle);

    if (!(
      Entities::is_light_component_valid(light_component) &&
      Entities::is_spatial_component_valid(spatial_component)
    )) {
      continue;
    }

    if (light_component->type == LightType::point) {
      light_component->color.b = ((real32)sin(t) + 1.0f) / 2.0f * 50.0f;
    }

    // For the sun! :)
    if (light_component->type == LightType::directional) {
      spatial_component->position = camera_position +
        -light_component->direction * Renderer::DIRECTIONAL_LIGHT_DISTANCE;
    }
  }
}


uint32 EntitySets::get_anim_channel_position_index_for_animation_timepoint(
  AnimChannel *anim_channel, real64 animation_timepoint
) {
  uint32 idx = anim_channel->last_position_key;
  do {
    if (
      animation_timepoint > anim_channel->position_keys[idx].time &&
      animation_timepoint < anim_channel->position_keys[idx + 1].time
    ) {
      anim_channel->last_position_key = idx;
      return idx;
    }
    idx++;
    if (idx == anim_channel->n_position_keys - 1) {
      idx = 0;
    }
  } while (idx != anim_channel->last_position_key);
  return 0;
}


uint32 EntitySets::get_anim_channel_rotation_index_for_animation_timepoint(
  AnimChannel *anim_channel, real64 animation_timepoint
) {
  uint32 idx = anim_channel->last_rotation_key;
  do {
    if (
      animation_timepoint > anim_channel->rotation_keys[idx].time &&
      animation_timepoint < anim_channel->rotation_keys[idx + 1].time
    ) {
      anim_channel->last_rotation_key = idx;
      return idx;
    }
    idx++;
    if (idx == anim_channel->n_rotation_keys - 1) {
      idx = 0;
    }
  } while (idx != anim_channel->last_rotation_key);
  return 0;
}


uint32 EntitySets::get_anim_channel_scaling_index_for_animation_timepoint(
  AnimChannel *anim_channel, real64 animation_timepoint
) {
  uint32 idx = anim_channel->last_scaling_key;
  do {
    if (
      animation_timepoint > anim_channel->scaling_keys[idx].time &&
      animation_timepoint < anim_channel->scaling_keys[idx + 1].time
    ) {
      anim_channel->last_scaling_key = idx;
      return idx;
    }
    idx++;
    if (idx == anim_channel->n_scaling_keys - 1) {
      idx = 0;
    }
  } while (idx != anim_channel->last_scaling_key);
  return 0;
}


void EntitySets::make_bone_translation(
  AnimChannel *anim_channel, real64 animation_timepoint, glm::mat4 *translation
) {
  if (anim_channel->n_position_keys == 1) {
    *translation = glm::translate(
      glm::mat4(1.0f), anim_channel->position_keys[0].position
    );
    return;
  }

  uint32 idx_position = get_anim_channel_position_index_for_animation_timepoint(
    anim_channel, animation_timepoint
  );
  real64 prev_keyframe = anim_channel->position_keys[idx_position].time;
  real64 next_keyframe = anim_channel->position_keys[idx_position + 1].time;

  *translation = glm::translate(
    glm::mat4(1.0f),
    glm::mix(
      anim_channel->position_keys[idx_position].position,
      anim_channel->position_keys[idx_position + 1].position,
      (animation_timepoint - prev_keyframe) / (next_keyframe - prev_keyframe)
    )
  );
}


void EntitySets::make_bone_rotation(
  AnimChannel *anim_channel, real64 animation_timepoint, glm::mat4 *rotation
) {
  if (anim_channel->n_rotation_keys == 1) {
    *rotation = glm::toMat4(glm::normalize(anim_channel->rotation_keys[0].rotation));
    return;
  }

  uint32 idx_rotation = get_anim_channel_rotation_index_for_animation_timepoint(
    anim_channel, animation_timepoint
  );
  real64 prev_keyframe = anim_channel->rotation_keys[idx_rotation].time;
  real64 next_keyframe = anim_channel->rotation_keys[idx_rotation + 1].time;

  *rotation = glm::toMat4(glm::normalize(glm::slerp(
    anim_channel->rotation_keys[idx_rotation].rotation,
    anim_channel->rotation_keys[idx_rotation + 1].rotation,
    (real32)((animation_timepoint - prev_keyframe) / (next_keyframe - prev_keyframe))
  )));
}


void EntitySets::make_bone_scaling(
  AnimChannel *anim_channel, real64 animation_timepoint, glm::mat4 *scaling
) {
  if (anim_channel->n_scaling_keys == 1) {
    *scaling = glm::scale(
      glm::mat4(1.0f), anim_channel->scaling_keys[0].scale
    );
    return;
  }

  uint32 idx_scaling = get_anim_channel_scaling_index_for_animation_timepoint(
    anim_channel, animation_timepoint
  );
  real64 prev_keyframe = anim_channel->scaling_keys[idx_scaling].time;
  real64 next_keyframe = anim_channel->scaling_keys[idx_scaling + 1].time;

  *scaling = glm::scale(
    glm::mat4(1.0f),
    glm::mix(
      anim_channel->scaling_keys[idx_scaling].scale,
      anim_channel->scaling_keys[idx_scaling + 1].scale,
      (animation_timepoint - prev_keyframe) / (next_keyframe - prev_keyframe)
    )
  );
}


void EntitySets::make_bone_matrices(
  AnimationComponent *animation_component,
  real64 t
) {
  glm::mat4 local_bone_matrices[MAX_N_BONES];

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
    glm::mat4 scaling = glm::mat4(1.0f);

    if (idx > 0) {
      parent_transform = local_bone_matrices[bone->idx_parent];
    }

    AnimChannel *anim_channel = &animation->anim_channels[idx];

    if (anim_channel->n_position_keys > 0) {
      make_bone_translation(anim_channel, animation_timepoint, &translation);
    }

    if (anim_channel->n_rotation_keys > 0) {
      make_bone_rotation(anim_channel, animation_timepoint, &rotation);
    }

    if (anim_channel->n_scaling_keys > 0) {
      make_bone_scaling(anim_channel, animation_timepoint, &scaling);
    }

    glm::mat4 anim_transform = translation * rotation * scaling;
    local_bone_matrices[idx] = parent_transform * anim_transform;
    animation_component->bone_matrices[idx] =
      animation_component->scene_root_transform *
      local_bone_matrices[idx] *
      bone->offset *
      animation_component->inverse_scene_root_transform;
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
    } else if (bone_matrices && Str::eq(uniform_name, "bone_matrices[0]")) {
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
    DrawableComponent *drawable = drawable_component_set->components.get(idx);

    if (!Models::is_drawable_component_valid(drawable)) {
      continue;
    }

    if (!(render_pass & drawable->target_render_pass)) {
      continue;
    }

#if 0
    Entity *entity = entity_set->entities.get(idx);
    log_info("Drawing %s", entity->debug_name);
#endif

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
    glm::mat4 *bone_matrices = nullptr;

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
        bone_matrices = animation_component->bone_matrices;
      }
    }

    draw(
      render_mode,
      drawable_component_set,
      &drawable->mesh,
      material,
      &model_matrix,
      &model_normal_matrix,
      bone_matrices,
      standard_depth_shader_asset
    );
  }
}
