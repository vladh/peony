State* init_state(
  State *state,
  MemoryPool *asset_memory_pool,
  MemoryPool *entity_memory_pool,
  WindowInfo window_info
) {
  state->window_info = window_info;

  state->background_color = glm::vec4(0.81f, 0.93f, 1.00f, 1.0f);

  state->shader_assets = Array<ShaderAsset>(
    asset_memory_pool, 512, "shader_assets"
  );
  state->model_assets = Array<ModelAsset>(
    asset_memory_pool, 512, "model_assets"
  );

  state->heading_opacity = 0.0f;
  state->heading_text = "";
  state->heading_fadeout_duration = 1.0f;
  state->heading_fadeout_delay = 2.0f;

  state->task_queue = Queue<Task>(entity_memory_pool, 128, "task_queue");
  state->cube_shadowmap_width = state->window_info.width;
  state->cube_shadowmap_height = state->window_info.width;
  state->texture_shadowmap_width = (uint32)(2560.0f * 4.0f);
  state->texture_shadowmap_height = (uint32)(1440.0f * 4.0f);
  state->shadowmap_near_clip_dist = 0.05f;
  state->shadowmap_far_clip_dist = 200.0f;
  state->dir_light_angle = PI32 / 4.0f;

  state->entity_set = {
    .entities = Array<Entity>(
      entity_memory_pool, 4096, "entities", true
    )
  };
  state->drawable_component_set = {
    .components = Array<DrawableComponent>(
      entity_memory_pool, 4096, "drawable_components", true
    )
  };
  state->light_component_set = {
    .components = Array<LightComponent>(
      entity_memory_pool, 4096, "light_components", true
    )
  };
  state->spatial_component_set = {
    .components = Array<SpatialComponent>(
      entity_memory_pool, 4096, "spatial_components", true
    )
  };
  state->behavior_component_set = {
    .components = Array<BehaviorComponent>(
      entity_memory_pool, 4096, "behavior_components", true
    )
  };

  return state;
}
