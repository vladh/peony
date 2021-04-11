void World::get_entity_text_representation(
  char *text,
  State *state,
  Entity *entity,
  uint8 depth
) {
  EntityHandle handle = entity->handle;
  SpatialComponent *spatial_component =
    state->spatial_component_set.components[handle];

  // Children will be drawn under their parents.
  if (
    depth == 0 &&
    Entities::is_spatial_component_valid(spatial_component) &&
    spatial_component->parent_entity_handle != Entity::no_entity_handle
  ) {
    return;
  }

  bool32 has_spatial_component = Entities::is_spatial_component_valid(
    spatial_component
  );
  bool32 has_drawable_component = Models::is_drawable_component_valid(
    state->drawable_component_set.components[handle]
  );
  bool32 has_light_component = Entities::is_light_component_valid(
    state->light_component_set.components[handle]
  );
  bool32 has_behavior_component = Entities::is_behavior_component_valid(
    state->behavior_component_set.components[handle]
  );
  bool32 has_animation_component = Entities::is_animation_component_valid(
    state->animation_component_set.components[handle]
  );

  for (uint8 level = 0; level < depth; level++) {
    strcat(text, "  ");
  }

  strcat(text, "- ");
  strcat(text, entity->debug_name);

  strcat(text, "@");
  // Because NUM_TO_STR only has 2048 entries
  assert(entity->handle < 2048);
  strcat(text, NUM_TO_STR[entity->handle]);

  if (
    !has_spatial_component &&
    !has_drawable_component &&
    !has_light_component &&
    !has_behavior_component &&
    !has_animation_component
  ) {
    strcat(text, " (orphan)");
  }

  if (has_spatial_component) {
    strcat(text, " +S");
  }
  if (has_drawable_component) {
    strcat(text, " +D");
  }
  if (has_light_component) {
    strcat(text, " +L");
  }
  if (has_behavior_component) {
    strcat(text, " +B");
  }
  if (has_animation_component) {
    strcat(text, " +A");
  }

  if (Entities::is_spatial_component_valid(spatial_component)) {
    // NOTE: This is super slow lol.
    uint32 n_children_found = 0;
    for_each (child_spatial_component, state->spatial_component_set.components) {
      if (
        child_spatial_component->parent_entity_handle ==
          spatial_component->entity_handle
      ) {
        n_children_found++;
        if (n_children_found > 5) {
          continue;
        }
        EntityHandle child_handle = child_spatial_component->entity_handle;
        Entity *child_entity = state->entity_set.entities[child_handle];

        if (text[strlen(text) - 1] != '\n') {
          strcat(text, "\n");
        }
        get_entity_text_representation(text, state, child_entity, depth + 1);
      }
    }
    if (n_children_found > 5) {
      for (uint8 level = 0; level < (depth + 1); level++) {
        strcat(text, "  ");
      }
      strcat(text, "(and ");
      strcat(text, NUM_TO_STR[n_children_found - 5]);
      strcat(text, " more)");
    }
  }

  if (text[strlen(text) - 1] != '\n') {
    strcat(text, "\n");
  }
}


void World::get_scene_text_representation(char *text, State *state) {
  text[0] = '\0';

  for_each (entity, state->entity_set.entities) {
    get_entity_text_representation(text, state, entity, 0);
  }

  if (text[strlen(text) - 1] == '\n') {
    text[strlen(text) - 1] = '\0';
  }
}


void World::get_materials_text_representation(char *text, State *state) {
  text[0] = '\0';

  strcat(text, "Internal:\n");

  uint32 idx = 0;
  for_each (material, state->materials) {
    strcat(text, "- ");
    strcat(text, material->name);
    strcat(text, "\n");
    if (idx == state->first_non_internal_material_idx - 1) {
      strcat(text, "Non-internal: \n");
    }
    idx++;
  }

  if (text[strlen(text) - 1] == '\n') {
    text[strlen(text) - 1] = '\0';
  }
}


void World::update_light_position(State *state, real32 amount) {
  for_each (light_component, state->light_component_set.components) {
    if (light_component->type == LightType::directional) {
      state->dir_light_angle += amount;
      light_component->direction = glm::vec3(
        sin(state->dir_light_angle), -cos(state->dir_light_angle), 0.0f
      );
      break;
    }
  }
}


void World::create_entity_loader_from_entity_template(
  EntityTemplate *entity_template,
  EntitySet *entity_set,
  EntityLoaderSet *entity_loader_set,
  State *state,
  MemoryPool *memory_pool
) {
  // TODO: Figure out parent system.
  Entity *entity = EntitySets::add_entity_to_set(
    entity_set, entity_template->entity_debug_name
  );

  EntityLoader *entity_loader = entity_loader_set->loaders[entity->handle];
  bool32 did_init_loader = false;

  if (Str::is_empty(entity_template->builtin_model_name)) {
    Models::init_entity_loader(
      entity_loader,
      ModelSource::file,
      entity_template->entity_debug_name,
      entity_template->model_path,
      entity_template->render_pass,
      entity->handle
    );
    did_init_loader = true;
  } else {
    if (strcmp(entity_template->builtin_model_name, "axes") == 0) {
      Models::init_entity_loader(
        entity_loader,
        ModelSource::data,
        (Vertex*)AXES_VERTICES, 6,
        nullptr, 0,
        entity_template->entity_debug_name,
        GL_LINES,
        entity_template->render_pass,
        entity->handle
      );
      did_init_loader = true;
    } else if (strcmp(entity_template->builtin_model_name, "ocean") == 0) {
      uint32 ocean_n_vertices;
      uint32 ocean_n_indices;
      Vertex *ocean_vertex_data;
      uint32 *ocean_index_data;

      Util::make_plane(
        memory_pool,
        200, 200,
        800, 800,
        &ocean_n_vertices, &ocean_n_indices,
        &ocean_vertex_data, &ocean_index_data
      );

      Models::init_entity_loader(
        entity_loader,
        ModelSource::data,
        ocean_vertex_data, ocean_n_vertices,
        ocean_index_data, ocean_n_indices,
        entity_template->entity_debug_name,
        GL_TRIANGLES,
        entity_template->render_pass,
        entity->handle
      );
      did_init_loader = true;
    } else {
      log_fatal(
        "Could not find builtin model: %s", entity_template->builtin_model_name
      );
    }
  }

  if (!did_init_loader) {
    log_fatal("Did not find any models to init entity loader with.");
  }

  // TODO: Add support for parents! Forgot about this.
  memcpy(
    &entity_loader->spatial_component,
    &entity_template->spatial_component,
    sizeof(SpatialComponent)
  );

  memcpy(
    &entity_loader->light_component,
    &entity_template->light_component,
    sizeof(LightComponent)
  );

  memcpy(
    &entity_loader->behavior_component,
    &entity_template->behavior_component,
    sizeof(BehaviorComponent)
  );

  assert(
    sizeof(entity_loader->material_names) == sizeof(entity_template->material_names)
  );
  memcpy(
    entity_loader->material_names,
    entity_template->material_names,
    sizeof(entity_template->material_names)
  );
  entity_loader->n_materials = entity_template->n_materials;
}


void World::create_internal_materials(State *state) {
  MemoryPool temp_memory_pool = {};

  // unknown
  {
    Material *material = Materials::init_material(
      state->materials.push(), "unknown"
    );
    Shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "unknown", ShaderType::standard,
      "base.vert", "unknown.frag", ""
    );
  }

  // lighting
  {
    Material *material = Materials::init_material(
      state->materials.push(), "lighting"
    );
    Shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "lighting", ShaderType::standard,
      "lighting.vert", "lighting.frag", ""
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.g_position_texture, "g_position_texture"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.g_normal_texture, "g_normal_texture"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.g_albedo_texture, "g_albedo_texture"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.g_pbr_texture, "g_pbr_texture"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.cube_shadowmaps_texture, "cube_shadowmaps"
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.texture_shadowmaps_texture, "texture_shadowmaps"
    );
  }

#if USE_BLOOM
  // preblur
  {
    Material *material = Materials::init_material(
      state->materials.push(), "preblur"
    );
    Shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "blur", ShaderType::standard,
      "blur.vert", "blur.frag", ""
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.l_bright_color_texture, "source_texture"
    );
  }

  // blur1
  {
    Material *material = Materials::init_material(
      state->materials.push(), "blur1"
    );
    Shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "blur", ShaderType::standard,
      "blur.vert", "blur.frag", ""
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.blur2_texture, "source_texture"
    );
  }

  // blur2
  {
    Material *material = Materials::init_material(
      state->materials.push(), "blur2"
    );
    Shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "blur", ShaderType::standard,
      "blur.vert", "blur.frag", ""
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.blur1_texture, "source_texture"
    );
  }
#endif

  // postprocessing
  {
    Material *material = Materials::init_material(
      state->materials.push(), "postprocessing"
    );
    Shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "postprocessing", ShaderType::standard,
      "postprocessing.vert", "postprocessing.frag", ""
    );
    Materials::add_texture_to_material(
      material, *state->builtin_textures.l_color_texture, "l_color_texture"
    );
#if USE_BLOOM
    Materials::add_texture_to_material(
      material, *state->builtin_textures.blur2_texture, "bloom_texture"
    );
#endif
    // Uncomment to use fog.
    /* Materials::add_texture_to-material( */
    /*   material, *state->l_depth_texture, "l_depth_texture" */
    /* ); */
  }

  // skysphere
  {
    Material *material = Materials::init_material(
      state->materials.push(), "skysphere"
    );
    Shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "skysphere", ShaderType::standard,
      "skysphere.vert", "skysphere.frag", ""
    );
  }


  // We've created all internal materials, so we will mark the next position
  // in the array of materials, so we know where non-internal materials start.
  state->first_non_internal_material_idx = state->materials.length;

  Memory::destroy_memory_pool(&temp_memory_pool);
}


void World::create_internal_entities(State *state) {
  MemoryPool temp_memory_pool = {};

  Shaders::init_shader_asset(
    &state->standard_depth_shader_asset,
    &temp_memory_pool,
    "standard_depth", ShaderType::depth,
    "standard_depth.vert", "standard_depth.frag",
    "standard_depth.geom"
  );

  uint32 skysphere_n_vertices;
  uint32 skysphere_n_indices;
  Vertex *skysphere_vertex_data;
  uint32 *skysphere_index_data;

  Util::make_sphere(
    &temp_memory_pool,
    64, 64,
    &skysphere_n_vertices, &skysphere_n_indices,
    &skysphere_vertex_data, &skysphere_index_data
  );

  // Lighting screenquad
  {
    Entity *entity = EntitySets::add_entity_to_set(
      &state->entity_set, "screenquad_lighting"
    );
    EntityLoader *entity_loader =
      state->entity_loader_set.loaders[entity->handle];
    Models::init_entity_loader(
      entity_loader,
      ModelSource::data,
      (Vertex*)SCREENQUAD_VERTICES, 6,
      nullptr, 0,
      "screenquad_lighting",
      GL_TRIANGLES,
      RenderPass::lighting,
      entity->handle
    );
    strcpy(entity_loader->material_names[0], "lighting");
    entity_loader->n_materials = 1;
  }

#if USE_BLOOM
  // Preblur screenquad
  {
    Entity *entity = EntitySets::add_entity_to_set(
      &state->entity_set, "screenquad_preblur"
    );
    EntityLoader *entity_loader = state->entity_loader_set.loaders.get(
      entity->handle
    );
    Models::init_entity_loader(
      entity_loader,
      ModelSource::data,
      (Vertex*)SCREENQUAD_VERTICES, 6,
      nullptr, 0,
      "screenquad_preblur",
      GL_TRIANGLES,
      RenderPass::preblur,
      entity->handle
    );
    strcpy(entity_loader->material_names[0], "preblur");
    entity_loader->n_materials = 1;
  }

  // Blur 1 screenquad
  {
    Entity *entity = EntitySets::add_entity_to_set(
      &state->entity_set, "screenquad_blur1"
    );
    EntityLoader *entity_loader = state->entity_loader_set.loaders.get(
      entity->handle
    );
    Models::init_entity_loader(
      entity_loader,
      ModelSource::data,
      (Vertex*)SCREENQUAD_VERTICES, 6,
      nullptr, 0,
      "screenquad_blur1",
      GL_TRIANGLES,
      RenderPass::blur1,
      entity->handle
    );
    strcpy(entity_loader->material_names[0], "blur1");
    entity_loader->n_materials = 1;
  }

  // Blur 2 screenquad
  {
    Entity *entity = EntitySets::add_entity_to_set(
      &state->entity_set, "screenquad_blur2"
    );
    EntityLoader *entity_loader = state->entity_loader_set.loaders.get(
      entity->handle
    );
    Models::init_entity_loader(
      entity_loader,
      ModelSource::data,
      (Vertex*)SCREENQUAD_VERTICES, 6,
      nullptr, 0,
      "screenquad_blur2",
      GL_TRIANGLES,
      RenderPass::blur2,
      entity->handle
    );
    strcpy(entity_loader->material_names[0], "blur2");
    entity_loader->n_materials = 1;
  }
#endif

  // Postprocessing screenquad
  {
    Entity *entity = EntitySets::add_entity_to_set(
      &state->entity_set, "screenquad_postprocessing"
    );
    EntityLoader *entity_loader =
      state->entity_loader_set.loaders[entity->handle];
    Models::init_entity_loader(
      entity_loader,
      ModelSource::data,
      (Vertex*)SCREENQUAD_VERTICES, 6,
      nullptr, 0,
      "screenquad_postprocessing",
      GL_TRIANGLES,
      RenderPass::postprocessing,
      entity->handle
    );
    strcpy(entity_loader->material_names[0], "postprocessing");
    entity_loader->n_materials = 1;
  }

  // Skysphere
  {
#if 1
    Entity *entity = EntitySets::add_entity_to_set(
      &state->entity_set, "skysphere"
    );
    EntityLoader *entity_loader =
      state->entity_loader_set.loaders[entity->handle];
    Models::init_entity_loader(
      entity_loader,
      ModelSource::data,
      skysphere_vertex_data, skysphere_n_vertices,
      skysphere_index_data, skysphere_n_indices,
      "skysphere",
      GL_TRIANGLE_STRIP,
      RenderPass::forward_skybox,
      entity->handle
    );
    entity_loader->spatial_component = {
      .entity_handle = entity->handle,
      .position = glm::vec3(0.0f),
      .rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
      .scale = glm::vec3(75.0f),
    };
    strcpy(entity_loader->material_names[0], "skysphere");
    entity_loader->n_materials = 1;
#endif
  }

  // We've created all internal entities, so we will mark the next position
  // in the EntitySet, to know that that position is where the non-internal
  // entities start.
  state->entity_set.first_non_internal_handle = state->entity_set.next_handle;

  Memory::destroy_memory_pool(&temp_memory_pool);
}


void World::destroy_non_internal_materials(State *state) {
  for (
    uint32 idx = state->first_non_internal_material_idx;
    idx < state->materials.length;
    idx++
  ) {
    Materials::destroy_material(state->materials[idx]);
  }

  state->materials.delete_elements_after_index(
    state->first_non_internal_material_idx
  );
}


void World::destroy_non_internal_entities(State *state) {
  for (
    uint32 idx = state->entity_set.first_non_internal_handle;
    idx < state->entity_set.entities.length;
    idx++
  ) {
    Models::destroy_drawable_component(
      state->drawable_component_set.components[idx]
    );
    memset(state->entity_set.entities[idx], 0, sizeof(Entity));
    memset(state->light_component_set.components[idx], 0, sizeof(LightComponent));
    memset(state->spatial_component_set.components[idx], 0, sizeof(SpatialComponent));
    memset(state->drawable_component_set.components[idx], 0, sizeof(DrawableComponent));
    memset(state->behavior_component_set.components[idx], 0, sizeof(BehaviorComponent));
  }

  state->entity_set.next_handle = state->entity_set.first_non_internal_handle;
  state->entity_set.entities.delete_elements_after_index(
    state->entity_set.first_non_internal_handle
  );
  state->light_component_set.components.delete_elements_after_index(
    state->entity_set.first_non_internal_handle
  );
  state->spatial_component_set.components.delete_elements_after_index(
    state->entity_set.first_non_internal_handle
  );
  state->drawable_component_set.components.delete_elements_after_index(
    state->entity_set.first_non_internal_handle
  );
  state->behavior_component_set.components.delete_elements_after_index(
    state->entity_set.first_non_internal_handle
  );
}

void World::load_scene(
  const char *scene_path,
  State *state
) {
  // Get some memory for everything we need
  MemoryPool temp_memory_pool = {};
  EntityTemplate *entity_templates = (EntityTemplate*)Memory::push(
    &temp_memory_pool,
    sizeof(EntityTemplate) * PeonyFileParser::MAX_N_FILE_ENTRIES,
    "entity_templates"
  );
  MaterialTemplate *material_templates = (MaterialTemplate*)Memory::push(
    &temp_memory_pool,
    sizeof(MaterialTemplate) * PeonyFileParser::MAX_N_FILE_ENTRIES,
    "material_templates"
  );
  StackArray<char[MAX_TOKEN_LENGTH], MAX_N_MATERIALS> used_materials;

  // Get EntityTemplates and MaterialTemplates
  uint32 n_entities = PeonyFileParser::parse_scene_file(
    scene_path,
    entity_templates,
    &used_materials
  );

  for_range (0, used_materials.length) {
    material_templates[idx] = {};
    char path[MAX_PATH];
    PeonyFileParser::get_material_path(path, *used_materials[idx]);
    PeonyFileParser::parse_material_file(path, &material_templates[idx]);
  }

  // Create materials
  for_range (0, used_materials.length) {
    Materials::create_material_from_template(
      state->materials.push(),
      &material_templates[idx],
      &state->builtin_textures,
      &temp_memory_pool
    );
  }

  // Create entities
  for_range (0, n_entities) {
    create_entity_loader_from_entity_template(
      &entity_templates[idx],
      &state->entity_set,
      &state->entity_loader_set,
      state,
      &temp_memory_pool
    );
  }

  // Clean up
  Memory::destroy_memory_pool(&temp_memory_pool);
}


void World::init(State *state) {
  create_internal_materials(state);
  create_internal_entities(state);
}


bool32 World::check_all_entities_loaded(State *state) {
  bool are_all_done_loading = true;

  for_each (material, state->materials) {
    bool is_done_loading = Materials::prepare_material_and_check_if_done(
      material,
      &state->persistent_pbo,
      &state->texture_name_pool,
      &state->task_queue
    );
    if (!is_done_loading) {
      are_all_done_loading = false;
    }
  }

  for_each (entity_loader, state->entity_loader_set.loaders) {
    if (!Entities::is_entity_loader_valid(entity_loader)) {
      continue;
    }
    bool is_done_loading = Models::prepare_model_and_check_if_done(
      entity_loader,
      &state->persistent_pbo,
      &state->texture_name_pool,
      &state->task_queue,
      &state->entity_set,
      &state->drawable_component_set,
      &state->spatial_component_set,
      &state->light_component_set,
      &state->behavior_component_set,
      &state->animation_component_set,
      &state->bone_matrix_pool
    );
    if (!is_done_loading) {
      are_all_done_loading = false;
    }
  }

  return are_all_done_loading;
}


void World::update(State *state) {
  Cameras::update_matrices(
    state->camera_active,
    state->window_info.width,
    state->window_info.height
  );
  state->is_world_loaded = check_all_entities_loaded(state);

  EntitySets::update_animation_components(
    &state->animation_component_set,
    &state->spatial_component_set,
    state->t,
    &state->bone_matrix_pool
  );

  EntitySets::update_behavior_components(
    &state->behavior_component_set,
    &state->spatial_component_set,
    state->t
  );

  EntitySets::update_light_components(
    &state->light_component_set,
    &state->spatial_component_set,
    state->t,
    state->camera_active->position
  );
}
