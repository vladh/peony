State* init_state(
  State *state,
  MemoryPool *asset_memory_pool,
  WindowInfo window_info
) {
  state->window_info = window_info;

  state->background_color = glm::vec4(0.81f, 0.93f, 1.00f, 1.0f);

  state->materials = Array<Material>(
    asset_memory_pool, MAX_N_MATERIALS, "materials"
  );
  state->model_loaders = Array<ModelLoader>(
    asset_memory_pool, MAX_N_MODELS, "model_loaders"
  );

  state->task_queue = Queue<Task>(asset_memory_pool, 128, "task_queue");

  state->builtin_textures = {
    .cube_shadowmap_width = state->window_info.width,
    .cube_shadowmap_height = state->window_info.width,
    .texture_shadowmap_width = (uint32)(2560.0f * 4.0f),
    .texture_shadowmap_height = (uint32)(1440.0f * 4.0f),
    .shadowmap_near_clip_dist = 0.05f,
    .shadowmap_far_clip_dist = 200.0f,
  };

  state->dir_light_angle = DEG_TO_RAD32(55);

  state->entity_loader_set = {
    .loaders = Array<EntityLoader>(
      asset_memory_pool, MAX_N_ENTITIES, "entity_loaders", true, 1
    )
  };
  state->entity_set = {
    .entities = Array<Entity>(
      asset_memory_pool, MAX_N_ENTITIES, "entities", true, 1
    )
  };
  state->drawable_component_set = {
    .components = Array<DrawableComponent>(
      asset_memory_pool, MAX_N_ENTITIES, "drawable_components", true, 1
    )
  };
  state->light_component_set = {
    .components = Array<LightComponent>(
      asset_memory_pool, MAX_N_ENTITIES, "light_components", true, 1
    )
  };
  state->spatial_component_set = {
    .components = Array<SpatialComponent>(
      asset_memory_pool, MAX_N_ENTITIES, "spatial_components", true, 1
    )
  };
  state->behavior_component_set = {
    .components = Array<BehaviorComponent>(
      asset_memory_pool, MAX_N_ENTITIES, "behavior_components", true, 1
    )
  };
  state->animation_component_set = {
    .components = Array<AnimationComponent>(
      asset_memory_pool, MAX_N_ENTITIES, "animation_components", true, 1
    )
  };
  state->bone_matrix_pool.bone_matrices = Array<glm::mat4>(
    asset_memory_pool,
    MAX_N_ANIMATED_MODELS * MAX_N_BONES * MAX_N_ANIMATIONS * MAX_N_ANIM_KEYS,
    "bone_matrices",
    true
  );
  state->bone_matrix_pool.times = Array<real64>(
    asset_memory_pool,
    MAX_N_ANIMATED_MODELS * MAX_N_BONES * MAX_N_ANIMATIONS * MAX_N_ANIM_KEYS,
    "bone_matrix_times",
    true
  );

  return state;
}
