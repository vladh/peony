uint32 EntitySets::push_to_bone_matrix_pool(BoneMatrixPool *pool) {
  return pool->n_bone_matrix_sets++;
}


glm::mat4* EntitySets::get_bone_matrix(
  BoneMatrixPool *pool,
  uint32 idx,
  uint32 idx_bone,
  uint32 idx_anim_key
) {
  return pool->bone_matrices[
    idx * MAX_N_ANIM_KEYS * MAX_N_BONES +
    idx_anim_key * MAX_N_BONES +
    idx_bone
  ];
}


real64* EntitySets::get_bone_matrix_time(
  BoneMatrixPool *pool,
  uint32 idx,
  uint32 idx_bone,
  uint32 idx_anim_key
) {
  return pool->times[
    idx * MAX_N_ANIM_KEYS * MAX_N_BONES +
    idx_anim_key * MAX_N_BONES +
    idx_bone
  ];
}


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
  Entity *new_entity = entity_set->entities[new_handle];
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
    SpatialComponent *parent = spatial_component_set->components[
      spatial_component->parent_entity_handle
    ];
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
  real64 t,
  BoneMatrixPool *bone_matrix_pool
) {
  for_each (animation_component, animation_component_set->components) {
    if (!Entities::is_animation_component_valid(animation_component)) {
      continue;
    }

    Animation *animation = &animation_component->animations[0];

    for_range_named (idx_bone, 0, animation_component->n_bones) {
      Bone *bone = &animation_component->bones[idx_bone];

      // If we have no anim keys, just return the identity matrix.
      if (bone->n_anim_keys == 0) {
        animation_component->bone_matrices[idx_bone] = glm::mat4(1.0f);

      // If we only have one anim key, just return that.
      } else if (bone->n_anim_keys == 1) {
        animation_component->bone_matrices[idx_bone] = *EntitySets::get_bone_matrix(
          bone_matrix_pool,
          animation->idx_bone_matrix_set,
          idx_bone,
          0
        );

      // If we have multiple anim keys, find the right ones and interpolate.
      } else {
        real64 animation_timepoint = fmod(t, animation->duration);

        uint32 idx_anim_key = get_bone_matrix_anim_key_for_timepoint(
          bone_matrix_pool,
          animation_component,
          animation_timepoint,
          animation->idx_bone_matrix_set,
          idx_bone
        );

        real64 t0 = *get_bone_matrix_time(
          bone_matrix_pool, animation->idx_bone_matrix_set, idx_bone, idx_anim_key
        );
        glm::mat4 transform_t0 = *get_bone_matrix(
          bone_matrix_pool, animation->idx_bone_matrix_set, idx_bone, idx_anim_key
        );

        real64 t1 = *get_bone_matrix_time(
          bone_matrix_pool, animation->idx_bone_matrix_set, idx_bone, idx_anim_key + 1
        );
        glm::mat4 transform_t1 = *get_bone_matrix(
          bone_matrix_pool, animation->idx_bone_matrix_set, idx_bone, idx_anim_key + 1
        );

        real32 lerp_factor = (real32)((animation_timepoint - t0) / (t1 - t0));

        // NOTE: This is probably bad if we have scaling in our transform?
        glm::mat4 interpolated_matrix =
          (transform_t0 * (1.0f - lerp_factor)) + (transform_t1 * lerp_factor);

        animation_component->bone_matrices[idx_bone] = interpolated_matrix;
      }
    }
  }
}


void EntitySets::update_behavior_components(
  BehaviorComponentSet *behavior_component_set,
  SpatialComponentSet *spatial_component_set,
  real64 t
) {
  for_each (behavior_component, behavior_component_set->components) {
    if (!Entities::is_behavior_component_valid(behavior_component)) {
      continue;
    }

    EntityHandle entity_handle = behavior_component->entity_handle;

    SpatialComponent *spatial_component =
      spatial_component_set->components[entity_handle];
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
  for_each (light_component, light_component_set->components) {
    if (light_component->entity_handle == Entity::no_entity_handle) {
      continue;
    }

    SpatialComponent *spatial_component =
      spatial_component_set->components[light_component->entity_handle];

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


uint32 EntitySets::get_bone_matrix_anim_key_for_timepoint(
  BoneMatrixPool *bone_matrix_pool,
  AnimationComponent *animation_component,
  real64 animation_timepoint,
  uint32 idx_bone_matrix_set,
  uint32 idx_bone
) {
  Bone *bone = &animation_component->bones[idx_bone];
  assert(bone->n_anim_keys > 1);
  uint32 idx_anim_key = bone->last_anim_key;
  do {
    real64 t0 = *get_bone_matrix_time(
      bone_matrix_pool, idx_bone_matrix_set, idx_bone, idx_anim_key
    );
    real64 t1 = *get_bone_matrix_time(
      bone_matrix_pool, idx_bone_matrix_set, idx_bone, idx_anim_key + 1
    );
    if (animation_timepoint > t0 && animation_timepoint < t1) {
      bone->last_anim_key = idx_anim_key;
      return idx_anim_key;
    }
    idx_anim_key++;
    assert(idx_anim_key < bone->n_anim_keys);
    if (idx_anim_key == bone->n_anim_keys - 1) {
      idx_anim_key = 0;
    }
  } while (idx_anim_key != bone->last_anim_key);
  log_fatal("Could not find anim key.");
  return 0;
}


void EntitySets::make_bone_matrices_for_animation_bone(
  AnimationComponent *animation_component,
  aiNodeAnim *ai_channel,
  uint32 idx_animation,
  uint32 idx_bone,
  BoneMatrixPool *bone_matrix_pool
) {
  assert(ai_channel->mNumPositionKeys == ai_channel->mNumRotationKeys);
  assert(ai_channel->mNumPositionKeys == ai_channel->mNumScalingKeys);

  Bone *bone = &animation_component->bones[idx_bone];
  bone->n_anim_keys = ai_channel->mNumPositionKeys;

  for_range_named (idx_anim_key, 0, bone->n_anim_keys) {
    assert(
      ai_channel->mPositionKeys[idx_anim_key].mTime ==
        ai_channel->mRotationKeys[idx_anim_key].mTime
    );
    assert(
      ai_channel->mPositionKeys[idx_anim_key].mTime ==
        ai_channel->mScalingKeys[idx_anim_key].mTime
    );
    real64 anim_key_time = ai_channel->mPositionKeys[idx_anim_key].mTime;

    glm::mat4 *bone_matrix = get_bone_matrix(
      bone_matrix_pool,
      animation_component->animations[idx_animation].idx_bone_matrix_set,
      idx_bone,
      idx_anim_key
    );

    real64 *time = get_bone_matrix_time(
      bone_matrix_pool,
      animation_component->animations[idx_animation].idx_bone_matrix_set,
      idx_bone,
      idx_anim_key
    );

    glm::mat4 parent_transform = glm::mat4(1.0f);

    if (idx_bone > 0) {
      parent_transform = *get_bone_matrix(
        bone_matrix_pool,
        animation_component->animations[idx_animation].idx_bone_matrix_set,
        bone->idx_parent,
        idx_anim_key
      );
    }

    glm::mat4 translation = glm::translate(
      glm::mat4(1.0f),
      Util::aiVector3D_to_glm(&ai_channel->mPositionKeys[idx_anim_key].mValue)
    );
    glm::mat4 rotation = glm::toMat4(glm::normalize(
      Util::aiQuaternion_to_glm(&ai_channel->mRotationKeys[idx_anim_key].mValue)
    ));
    glm::mat4 scaling = glm::scale(
      glm::mat4(1.0f),
      Util::aiVector3D_to_glm(&ai_channel->mScalingKeys[idx_anim_key].mValue)
    );

    glm::mat4 anim_transform = translation * rotation * scaling;
    *bone_matrix = parent_transform * anim_transform;
    *time = anim_key_time;
  }
}


AnimationComponent* EntitySets::find_animation_component(
  SpatialComponent *spatial_component,
  SpatialComponentSet *spatial_component_set,
  AnimationComponentSet *animation_component_set
) {
  AnimationComponent *animation_component =
    animation_component_set->components[spatial_component->entity_handle];

  if (Entities::is_animation_component_valid(animation_component)) {
    return animation_component;
  }

  if (spatial_component->parent_entity_handle != Entity::no_entity_handle) {
    SpatialComponent *parent =
      spatial_component_set->components[spatial_component->parent_entity_handle];
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

  for_each (drawable_component, drawable_component_set->components) {
    if (!Models::is_drawable_component_valid(drawable_component)) {
      continue;
    }

    if (!(render_pass & drawable_component->target_render_pass)) {
      continue;
    }

#if 0
    Entity *entity = entity_set->entities.get(idx);
    log_info("Drawing %s", entity->debug_name);
#endif

    Material *material = Materials::get_material_by_name(
      materials, drawable_component->mesh.material_name
    );

    if (!material || material->state != MaterialState::complete) {
      material = Materials::get_material_by_name(materials, "unknown");
    }

    SpatialComponent *spatial =
      spatial_component_set->components[drawable_component->entity_handle];

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
      &drawable_component->mesh,
      material,
      &model_matrix,
      &model_normal_matrix,
      bone_matrices,
      standard_depth_shader_asset
    );
  }
}
