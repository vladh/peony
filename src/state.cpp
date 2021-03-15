State::State(
  MemoryPool *asset_memory_pool,
  MemoryPool *entity_memory_pool,
  WindowInfo window_info
) :
  is_manual_frame_advance_enabled(false),
  should_manually_advance_to_next_frame(false),
  should_stop(false),
  should_pause(false),
  should_hide_ui(false),
  window_info(window_info),

  t(0),
  dt(0),
  dt_hist_idx(0),
  last_fps(0),
  n_frames_since_start(0),

  is_cursor_disabled(true),
  should_limit_fps(false),
  should_use_wireframe(false),
  background_color(glm::vec4(0.81f, 0.93f, 1.00f, 1.0f)),

  shader_assets(
    Array<ShaderAsset>(asset_memory_pool, 512, "shader_assets")
  ),
  model_assets(
    Array<ModelAsset>(asset_memory_pool, 512, "model_assets")
  ),

  entities(
    Array<Entity>(
      entity_memory_pool, 4096, "entities", true
    )
  ),
  drawable_components(
    Array<DrawableComponent>(
      entity_memory_pool, 4096, "drawable_components", true
    )
  ),
  light_components(
    Array<LightComponent>(
      entity_memory_pool, 4096, "light_components", true
    )
  ),
  spatial_components(
    Array<SpatialComponent>(
      entity_memory_pool, 4096, "spatial_components", true
    )
  ),
  behavior_components(
    Array<BehaviorComponent>(
      entity_memory_pool, 4096, "behavior_components", true
    )
  ),
  // TODO: Remove.
  gui_state(),

  heading_opacity(0.0f),
  heading_text(""),
  heading_fadeout_duration(1.0f),
  heading_fadeout_delay(2.0f),

  task_queue(
    Queue<Task>(entity_memory_pool, 128, "task_queue")
  ),
  cube_shadowmap_width(this->window_info.width),
  cube_shadowmap_height(this->window_info.width),
  texture_shadowmap_width(
    (uint32)(2560.0f * 4.0f)
  ),
  texture_shadowmap_height(
    (uint32)(1440.0f * 4.0f)
  ),
  shadowmap_near_clip_dist(0.05f),
  shadowmap_far_clip_dist(200.0f),
  dir_light_angle(PI32 / 4.0f)
{
  EntitySets::init_entity_set(
    &this->entity_set, &this->entities
  );
  EntitySets::init_drawable_component_set(
    &this->drawable_component_set, &this->drawable_components
  );
  EntitySets::init_light_component_set(
    &this->light_component_set, &this->light_components
  );
  EntitySets::init_spatial_component_set(
    &this->spatial_component_set, &this->spatial_components
  );
  EntitySets::init_behavior_component_set(
    &this->behavior_component_set, &this->behavior_components
  );
}
