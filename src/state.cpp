State::State(
  Memory *memory,
  WindowInfo window_info
) :
  window_info(window_info),

  camera_main(CAMERA_PERSPECTIVE),
  camera_active(&this->camera_main),
  control(),

  t(0),
  dt(0),
  last_fps(0),

  is_cursor_disabled(true),
  should_limit_fps(false),
  background_color(glm::vec4(0.9f, 0.9f, 0.9f, 1.0f)),

  shader_assets(
    Array<ShaderAsset>(
      &memory->asset_memory_pool, 512, "shader_assets"
    )
  ),
  model_assets(
    Array<ModelAsset>(
      &memory->asset_memory_pool, 512, "model_assets"
    )
  ),

  entities(
    Array<Entity>(
      &memory->entity_memory_pool, 512, "entities"
    )
  ),
  drawable_components(
    Array<DrawableComponent>(
      &memory->entity_memory_pool, 512, "drawable_components"
    )
  ),
  light_components(
    Array<LightComponent>(
      &memory->entity_memory_pool, 512, "light_components"
    )
  ),
  spatial_components(
    Array<SpatialComponent>(
      &memory->entity_memory_pool, 512, "spatial_components"
    )
  ),

  entity_manager(&this->entities),
  drawable_component_manager(&this->drawable_components),
  light_component_manager(&this->light_components),
  spatial_component_manager(&this->spatial_components),
  text_manager(
    memory,
    this->shader_assets.push(),
    this->window_info.width,
    this->window_info.height
  ),

  lights(
    Array<EntityHandle>(
      &memory->entity_memory_pool, MAX_N_LIGHTS, "lights"
    )
  ),
  geese(
    Array<EntityHandle>(
      &memory->entity_memory_pool, 512, "geese"
    )
  ),
  spheres(
    Array<EntityHandle>(
      &memory->entity_memory_pool, 512, "spheres"
    )
  )
{
}
