void scene_init_objects(Memory *memory, State *state) {
  Entity *entity;

  // Lights
#if 1
  glm::vec3 light_direction = glm::vec3(
    sin(state->dir_light_angle), -sin(state->dir_light_angle), 0.0f
  );
  entity = state->entity_manager.add("sun");
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "light"),
    RenderPass::forward_nodepth
  );
  state->light_component_manager.add(
    entity->handle,
    LightType::directional,
    light_direction,
    glm::vec4(4.0f, 4.0f, 4.0f, 1.0f),
    glm::vec4(1.0f, 0.0f, 0.0f, 0.0f)
  );
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(-light_direction * DIRECTIONAL_LIGHT_DISTANCE),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(0.3f)
  );
  state->directional_lights.push(entity->handle);
#endif

#if 0
  entity = state->entity_manager.add("point_light");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(-7.0f, 3.0f, 0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(0.3f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "light"),
    RenderPass::forward_nodepth
  );
  state->light_component_manager.add(
    entity->handle,
    LightType::point,
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec4(200.0f, 0.0f, 0.0f, 1.0f),
    /* glm::vec4(1.0f, 0.09f, 0.032f, 0.0f) */
    glm::vec4(1.0f, 0.09f, 0.032f, 0.0f) * 2.0f
  );
  state->point_lights.push(entity->handle);
#endif

  // Ocean
#if 1
  entity = state->entity_manager.add("ocean");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(1.0f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "ocean"),
    RenderPass::forward_depth
  );
#endif

  // Temple
#if 1
  entity = state->entity_manager.add("temple");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(0.0f, 0.1f, 0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
    glm::vec3(0.1f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "temple"),
    RenderPass::deferred
  );
#endif

  // Test
  entity = state->entity_manager.add("test");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
    glm::vec3(1.0f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "light"),
    RenderPass::forward_depth | RenderPass::shadowcaster
  );
  state->behavior_component_manager.add(entity->handle, Behavior::test);

  // Rocks
#if 1
  entity = state->entity_manager.add("rocks");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(0.0f, -3.5f, 0.0f),
    glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
    glm::vec3(0.05f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "rocks"),
    RenderPass::deferred | RenderPass::shadowcaster
  );
#endif

  // Skysphere
#if 1
  entity = state->entity_manager.add("skysphere");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(75.0f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "skysphere"),
    RenderPass::forward_skybox
  );
#endif

  // Axes
#if 0
  entity = state->entity_manager.add("axes");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(0.0f, 0.1f, 0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(1.0f, 1.0f, 1.0f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "axes"),
    RenderPass::forward_nodepth
  );
#endif

  // Lighting screenquad
  entity = state->entity_manager.add("screenquad_lighting");
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "screenquad_lighting"),
    RenderPass::lighting
  );

  // Preblur screenquad
  entity = state->entity_manager.add("screenquad_preblur");
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "screenquad_preblur"),
    RenderPass::preblur
  );

  // Blur 1 screenquad
  entity = state->entity_manager.add("screenquad_blur1");
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "screenquad_blur1"),
    RenderPass::blur1
  );

  // Blur 2 screenquad
  entity = state->entity_manager.add("screenquad_blur2");
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "screenquad_blur2"),
    RenderPass::blur2
  );

  // Postprocessing screenquad
  entity = state->entity_manager.add("screenquad_postprocessing");
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "screenquad_postprocessing"),
    RenderPass::postprocessing
  );
}

#if 0
  // Geese
  entity = state->entity_manager.add("goose");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(-4.6f, 0.00f, -1.5f),
    glm::angleAxis(glm::radians(-30.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
    glm::vec3(0.2f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "goose"),
    RenderPass::deferred
  );

  // Floor
  entity = state->entity_manager.add("floor");
  state->spatial_component_manager.add(
    entity->handle,
    glm::vec3(0.0f),
    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
    glm::vec3(150.0f, 0.1f, 150.0f)
  );
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "floor"),
    RenderPass::deferred
  );
#endif
