#include "materials.hpp"
#include "shaders.hpp"
#include "internals.hpp"


void internals::create_internal_materials(State *state) {
  MemoryPool temp_memory_pool = {};

  // unknown
  {
    Material *material = materials::init_material(
      state->materials.push(), "unknown"
    );
    shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "unknown", ShaderType::standard,
      "base.vert", "unknown.frag", ""
    );
  }

  // lighting
  {
    Material *material = materials::init_material(
      state->materials.push(), "lighting"
    );
    shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "lighting", ShaderType::standard,
      "screenquad.vert", "lighting.frag", ""
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.g_position_texture, "g_position_texture"
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.g_normal_texture, "g_normal_texture"
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.g_albedo_texture, "g_albedo_texture"
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.g_pbr_texture, "g_pbr_texture"
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.shadowmaps_3d_texture, "shadowmaps_3d"
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.shadowmaps_2d_texture, "shadowmaps_2d"
    );
  }

#if USE_BLOOM
  // preblur
  {
    Material *material = materials::init_material(
      state->materials.push(), "preblur"
    );
    shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "blur", ShaderType::standard,
      "screenquad.vert", "blur.frag", ""
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.l_bright_color_texture, "source_texture"
    );
  }

  // blur1
  {
    Material *material = materials::init_material(
      state->materials.push(), "blur1"
    );
    shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "blur", ShaderType::standard,
      "screenquad.vert", "blur.frag", ""
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.blur2_texture, "source_texture"
    );
  }

  // blur2
  {
    Material *material = materials::init_material(
      state->materials.push(), "blur2"
    );
    shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "blur", ShaderType::standard,
      "screenquad.vert", "blur.frag", ""
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.blur1_texture, "source_texture"
    );
  }
#endif

  // postprocessing
  {
    Material *material = materials::init_material(
      state->materials.push(), "postprocessing"
    );
    shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "postprocessing", ShaderType::standard,
      "screenquad.vert", "postprocessing.frag", ""
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.l_color_texture, "l_color_texture"
    );
#if USE_BLOOM
    materials::add_texture_to_material(
      material, *state->builtin_textures.blur2_texture, "bloom_texture"
    );
#endif
    // Uncomment to use fog.
    /* materials::add_texture_to-material( */
    /*   material, *state->l_depth_texture, "l_depth_texture" */
    /* ); */
  }

  // renderdebug
  {
    Material *material = materials::init_material(
      state->materials.push(), "renderdebug"
    );
    shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "renderdebug", ShaderType::standard,
      "screenquad.vert", "renderdebug.frag", ""
    );

    materials::add_texture_to_material(
      material, *state->builtin_textures.g_position_texture, "g_position_texture"
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.g_normal_texture, "g_normal_texture"
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.g_albedo_texture, "g_albedo_texture"
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.g_pbr_texture, "g_pbr_texture"
    );

    materials::add_texture_to_material(
      material, *state->builtin_textures.g_position_texture, "l_color_texture"
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.l_color_texture, "l_bright_color_texture"
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.l_color_texture, "l_depth_texture"
    );

    materials::add_texture_to_material(
      material, *state->builtin_textures.l_color_texture, "blur1_texture"
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.l_color_texture, "blur2_texture"
    );

    materials::add_texture_to_material(
      material, *state->builtin_textures.shadowmaps_3d_texture, "shadowmaps_3d"
    );
    materials::add_texture_to_material(
      material, *state->builtin_textures.shadowmaps_2d_texture, "shadowmaps_2d"
    );
  }

  // skysphere
  {
    Material *material = materials::init_material(
      state->materials.push(), "skysphere"
    );
    shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "skysphere", ShaderType::standard,
      "skysphere.vert", "skysphere.frag", ""
    );
  }

  // We've created all internal materials, so we will mark the next position
  // in the array of materials, so we know where non-internal materials start.
  state->first_non_internal_material_idx = state->materials.length;

  memory::destroy_memory_pool(&temp_memory_pool);
}


void internals::create_internal_entities(State *state) {
  MemoryPool temp_memory_pool = {};

  shaders::init_shader_asset(
    &state->standard_depth_shader_asset,
    &temp_memory_pool,
    "standard_depth", ShaderType::depth,
    "standard_depth.vert", "standard_depth.frag",
    "standard_depth.geom"
  );

  // Lighting screenquad
  {
    Entity *entity = entities::add_entity_to_set(
      &state->entity_set, "screenquad_lighting"
    );
    ModelLoader *model_loader = state->model_loaders.push();
    EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
    models::init_model_loader(model_loader, "builtin:screenquad_lighting");
    models::init_entity_loader(
      entity_loader,
      "screenquad_lighting",
      "builtin:screenquad_lighting",
      RenderPass::lighting,
      entity->handle
    );
    models::add_material_to_model_loader(model_loader, "lighting");
  }

#if USE_BLOOM
  // Preblur screenquad
  {
    Entity *entity = entities::add_entity_to_set(
      &state->entity_set, "screenquad_preblur"
    );
    ModelLoader *model_loader = state->model_loaders.push();
    EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
    models::init_model_loader(model_loader "builtin:screenquad_preblur");
    models::init_entity_loader(
      entity_loader,
      "screenquad_preblur",
      "builtin:screenquad_preblur",
      RenderPass::preblur,
      entity->handle
    );
    models::add_material_to_model_loader(model_loader, "preblur");
  }

  // Blur 1 screenquad
  {
    Entity *entity = entities::add_entity_to_set(
      &state->entity_set, "screenquad_blur1"
    );
    ModelLoader *model_loader = state->model_loaders.push();
    EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
    models::init_model_loader(model_loader, "builtin:screenquad_blur1");
    models::init_entity_loader(
      entity_loader,
      "screenquad_blur1",
      "builtin:screenquad_blur1",
      RenderPass::blur1,
      entity->handle
    );
    models::add_material_to_model_loader(model_loader, "blur1");
  }

  // Blur 2 screenquad
  {
    Entity *entity = entities::add_entity_to_set(
      &state->entity_set, "screenquad_blur2"
    );
    ModelLoader *model_loader = state->model_loaders.push();
    EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
    models::init_model_loader(model_loader, "builtin:screenquad_blur2");
    models::init_entity_loader(
      entity_loader,
      "screenquad_blur2",
      "builtin:screenquad_blur2",
      RenderPass::blur2,
      entity->handle
    );
    models::add_material_to_model_loader(model_loader, "blur2");
  }
#endif

  // Postprocessing screenquad
  {
    Entity *entity = entities::add_entity_to_set(
      &state->entity_set, "screenquad_postprocessing"
    );
    ModelLoader *model_loader = state->model_loaders.push();
    EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
    models::init_model_loader(model_loader, "builtin:screenquad_postprocessing");
    models::init_entity_loader(
      entity_loader,
      "screenquad_postprocessing",
      "builtin:screenquad_postprocessing",
      RenderPass::postprocessing,
      entity->handle
    );
    models::add_material_to_model_loader(model_loader, "postprocessing");
  }

  // Debug screenquad
  {
    Entity *entity = entities::add_entity_to_set(
      &state->entity_set, "screenquad_renderdebug"
    );
    ModelLoader *model_loader = state->model_loaders.push();
    EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
    models::init_model_loader(model_loader, "builtin:screenquad_renderdebug");
    models::init_entity_loader(
      entity_loader,
      "screenquad_renderdebug",
      "builtin:screenquad_renderdebug",
      RenderPass::renderdebug,
      entity->handle
    );
    models::add_material_to_model_loader(model_loader, "renderdebug");
  }

  // Skysphere
  {
#if 1
    Entity *entity = entities::add_entity_to_set(
      &state->entity_set, "skysphere"
    );
    ModelLoader *model_loader = state->model_loaders.push();
    EntityLoader *entity_loader = state->entity_loader_set.loaders[entity->handle];
    models::init_model_loader(model_loader, "builtin:skysphere");
    models::init_entity_loader(
      entity_loader,
      "skysphere",
      "builtin:skysphere",
      RenderPass::forward_skybox,
      entity->handle
    );
    entity_loader->spatial_component = {
      .entity_handle = entity->handle,
      .position = v3(0.0f),
      .rotation = glm::angleAxis(radians(0.0f), v3(1.0f, 0.0f, 0.0f)),
      .scale = v3(75.0f),
    };
    models::add_material_to_model_loader(model_loader, "skysphere");
#endif
  }

  // We've created all internal entities, so we will mark the next position
  // in the EntitySet, to know that that position is where the non-internal
  // entities start.
  state->entity_set.first_non_internal_handle = state->entity_set.next_handle;

  memory::destroy_memory_pool(&temp_memory_pool);
}
