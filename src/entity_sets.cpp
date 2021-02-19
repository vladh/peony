Entities::EntityHandle EntitySets::make_handle(
  EntitySet *entity_set
) {
  return entity_set->last_handle++;
}


Entities::Entity* EntitySets::get_entity_from_set(
  EntitySet *entity_set,
  Entities::EntityHandle handle
) {
  return entity_set->entities->get(handle);
}


Entities::Entity* EntitySets::add_entity_to_set(
  EntitySet *entity_set,
  const char *debug_name
) {
  Entities::EntityHandle new_handle = make_handle(entity_set);
  Entities::Entity *new_entity = entity_set->entities->get(new_handle);
  new_entity->handle = new_handle;
  new_entity->debug_name = debug_name;
  return new_entity;
}


EntitySets::EntitySet* EntitySets::init_entity_set(
  EntitySet *entity_set,
  Array<Entities::Entity> *entities
) {
  entity_set->entities = entities;
  return entity_set;
}


Entities::LightComponent* EntitySets::get_light_component_from_set(
  LightComponentSet *light_component_set,
  Entities::EntityHandle handle
) {
  return light_component_set->components->get(handle);
}


EntitySets::LightComponentSet* EntitySets::init_light_component_set(
  LightComponentSet *light_component_set,
  Array<Entities::LightComponent> *components
) {
  light_component_set->components = components;
  return light_component_set;
}


Entities::SpatialComponent* EntitySets::get_spatial_component_from_set(
  SpatialComponentSet *spatial_component_set,
  Entities::EntityHandle handle
) {
  if (handle == Entities::Entity::no_entity_handle) {
    return nullptr;
  }
  return spatial_component_set->components->get(handle);
}


glm::mat4 EntitySets::make_model_matrix(
  SpatialComponentSet *spatial_component_set,
  Entities::SpatialComponent *spatial_component,
  ModelMatrixCache *cache
) {
  glm::mat4 model_matrix = glm::mat4(1.0f);

  if (spatial_component->parent_entity_handle != Entities::Entity::no_entity_handle) {
    Entities::SpatialComponent *parent = get_spatial_component_from_set(
      spatial_component_set,
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


EntitySets::SpatialComponentSet* EntitySets::init_spatial_component_set(
  SpatialComponentSet *spatial_component_set,
  Array<Entities::SpatialComponent> *components
) {
  spatial_component_set->components = components;
  return spatial_component_set;
}


Entities::DrawableComponent* EntitySets::get_drawable_component_from_set(
  DrawableComponentSet *drawable_component_set,
  Entities::EntityHandle handle
) {
  if (handle == Entities::Entity::no_entity_handle) {
    return nullptr;
  }
  return drawable_component_set->components->get(handle);
}


EntitySets::DrawableComponentSet* EntitySets::init_drawable_component_set(
  DrawableComponentSet *drawable_component_set,
  Array<Entities::DrawableComponent> *components
) {
  drawable_component_set->components = components;
  drawable_component_set->last_drawn_shader_program = 0;
  return drawable_component_set;
}


Entities::BehaviorComponent* EntitySets::get_behavior_component_from_set(
  EntitySets::BehaviorComponentSet *behavior_component_set,
  Entities::EntityHandle handle
) {
  if (handle == Entities::Entity::no_entity_handle) {
    return nullptr;
  }
  return behavior_component_set->components->get(handle);
}


EntitySets::BehaviorComponentSet* EntitySets::init_behavior_component_set(
  EntitySets::BehaviorComponentSet *behavior_component_set,
  Array<Entities::BehaviorComponent> *components
) {
  behavior_component_set->components = components;
  return behavior_component_set;
}


void EntitySets::draw(
  EntitySets::DrawableComponentSet *drawable_component_set,
  Models::Mesh *mesh,
  glm::mat4 *model_matrix,
  glm::mat3 *model_normal_matrix
) {
  Shaders::ShaderAsset *shader_asset = mesh->material->shader_asset;

  // If our shader program has changed since our last mesh, tell OpenGL about it.
  if (shader_asset->program != drawable_component_set->last_drawn_shader_program) {
    glUseProgram(shader_asset->program);
    drawable_component_set->last_drawn_shader_program = shader_asset->program;

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


void EntitySets::draw_in_depth_mode(
  EntitySets::DrawableComponentSet *drawable_component_set,
  Models::Mesh *mesh,
  glm::mat4 *model_matrix,
  glm::mat3 *model_normal_matrix,
  Shaders::ShaderAsset *standard_depth_shader_asset
) {
  Shaders::ShaderAsset *shader_asset = standard_depth_shader_asset;

  if (mesh->material->depth_shader_asset) {
    shader_asset = mesh->material->depth_shader_asset;
  }

  // If our shader program has changed since our last mesh, tell OpenGL about it.
  if (shader_asset->program != drawable_component_set->last_drawn_shader_program) {
    glUseProgram(shader_asset->program);
    drawable_component_set->last_drawn_shader_program = shader_asset->program;
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
  Renderer::RenderPassFlag render_pass,
  Renderer::RenderMode render_mode,
  Shaders::ShaderAsset *standard_depth_shader_asset
) {
  ModelMatrixCache cache = {glm::mat4(1.0f), nullptr};

  for (uint32 idx = 0; idx < drawable_component_set->components->size; idx++) {
    Entities::DrawableComponent *drawable = drawable_component_set->components->get(idx);

    if (!Entities::is_drawable_component_valid(drawable)) {
      continue;
    }

    if (!(render_pass & drawable->target_render_pass)) {
      continue;
    }

    Entities::SpatialComponent *spatial = get_spatial_component_from_set(
      spatial_component_set,
      drawable->entity_handle
    );

    glm::mat4 model_matrix = glm::mat4(1.0f);
    glm::mat3 model_normal_matrix = glm::mat3(1.0f);

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
    }

    if (render_mode == Renderer::RenderMode::depth) {
      draw_in_depth_mode(
        drawable_component_set,
        drawable->mesh,
        &model_matrix,
        &model_normal_matrix,
        standard_depth_shader_asset
      );
    } else {
      draw(
        drawable_component_set,
        drawable->mesh,
        &model_matrix,
        &model_normal_matrix
      );
    }
  }
}
