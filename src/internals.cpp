// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "engine.hpp"
#include "renderer.hpp"
#include "mats.hpp"
#include "shaders.hpp"
#include "internals.hpp"


void internals::create_internal_materials(
  EngineState *engine_state,
  renderer::State *renderer_state,
  mats::State *materials_state
) {
  MemoryPool temp_memory_pool = {};
  renderer::BuiltinTextures *builtin_textures = &renderer_state->builtin_textures;

  // unknown
  {
    mats::Material *material = mats::init_material(
      materials_state->materials.push(), "unknown"
    );
    shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "unknown", shaders::Type::standard,
      "base.vert", "unknown.frag", ""
    );
  }

  // lighting
  {
    mats::Material *material = mats::init_material(
      materials_state->materials.push(), "lighting"
    );
    shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "lighting", shaders::Type::standard,
      "screenquad.vert", "lighting.frag", ""
    );
    mats::add_texture_to_material(
      material, *builtin_textures->g_position_texture, "g_position_texture"
    );
    mats::add_texture_to_material(
      material, *builtin_textures->g_normal_texture, "g_normal_texture"
    );
    mats::add_texture_to_material(
      material, *builtin_textures->g_albedo_texture, "g_albedo_texture"
    );
    mats::add_texture_to_material(
      material, *builtin_textures->g_pbr_texture, "g_pbr_texture"
    );
    mats::add_texture_to_material(
      material, *builtin_textures->shadowmaps_3d_texture, "shadowmaps_3d"
    );
    mats::add_texture_to_material(
      material, *builtin_textures->shadowmaps_2d_texture, "shadowmaps_2d"
    );
  }

  #if USE_BLOOM
    // preblur
    {
      mats::Material *material = mats::init_material(
        materials_state->materials.push(), "preblur"
      );
      shaders::init_shader_asset(
        &material->shader_asset,
        &temp_memory_pool,
        "blur", shaders::Type::standard,
        "screenquad.vert", "blur.frag", ""
      );
      mats::add_texture_to_material(
        material,
        *builtin_textures->l_bright_color_texture, "source_texture"
      );
    }

    // blur1
    {
      mats::Material *material = mats::init_material(
        materials_state->materials.push(), "blur1"
      );
      shaders::init_shader_asset(
        &material->shader_asset,
        &temp_memory_pool,
        "blur", shaders::Type::standard,
        "screenquad.vert", "blur.frag", ""
      );
      mats::add_texture_to_material(
        material,
        *builtin_textures->blur2_texture, "source_texture"
      );
    }

    // blur2
    {
      mats::Material *material = mats::init_material(
        materials_state->materials.push(), "blur2"
      );
      shaders::init_shader_asset(
        &material->shader_asset,
        &temp_memory_pool,
        "blur", shaders::Type::standard,
        "screenquad.vert", "blur.frag", ""
      );
      mats::add_texture_to_material(
        material,
        *builtin_textures->blur1_texture, "source_texture"
      );
    }
  #endif

  // postprocessing
  {
    mats::Material *material = mats::init_material(
      materials_state->materials.push(), "postprocessing"
    );
    shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "postprocessing", shaders::Type::standard,
      "screenquad.vert", "postprocessing.frag", ""
    );
    mats::add_texture_to_material(
      material, *builtin_textures->l_color_texture, "l_color_texture"
    );

    #if USE_BLOOM
      mats::add_texture_to_material(
        material, *builtin_textures->blur2_texture, "bloom_texture"
      );
    #endif

    #if USE_FOG
      mats::add_texture_to_material(
        material,
        *renderer_state->l_depth_texture, "l_depth_texture"
      );
    #endif
  }

  // renderdebug
  {
    mats::Material *material = mats::init_material(
      materials_state->materials.push(), "renderdebug"
    );
    shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "renderdebug", shaders::Type::standard,
      "screenquad.vert", "renderdebug.frag", ""
    );

    mats::add_texture_to_material(
      material, *renderer_state->builtin_textures.g_position_texture, "g_position_texture"
    );
    mats::add_texture_to_material(
      material, *renderer_state->builtin_textures.g_normal_texture, "g_normal_texture"
    );
    mats::add_texture_to_material(
      material, *renderer_state->builtin_textures.g_albedo_texture, "g_albedo_texture"
    );
    mats::add_texture_to_material(
      material, *renderer_state->builtin_textures.g_pbr_texture, "g_pbr_texture"
    );

    mats::add_texture_to_material(
      material, *renderer_state->builtin_textures.l_color_texture, "l_color_texture"
    );
    mats::add_texture_to_material(
      material,
      *renderer_state->builtin_textures.l_bright_color_texture,
      "l_bright_color_texture"
    );

    #if USE_FOG
      mats::add_texture_to_material(
        material, *renderer_state->builtin_textures.l_depth_texture, "l_depth_texture"
      );
    #endif

    #if USE_BLOOM
      mats::add_texture_to_material(
        material, *renderer_state->builtin_textures.blur1_texture, "blur1_texture"
      );
      mats::add_texture_to_material(
        material, *renderer_state->builtin_textures.blur2_texture, "blur2_texture"
      );
    #endif

    mats::add_texture_to_material(
      material, *renderer_state->builtin_textures.shadowmaps_3d_texture, "shadowmaps_3d"
    );
    mats::add_texture_to_material(
      material, *renderer_state->builtin_textures.shadowmaps_2d_texture, "shadowmaps_2d"
    );
  }

  // skysphere
  {
    mats::Material *material = mats::init_material(
      materials_state->materials.push(), "skysphere"
    );
    shaders::init_shader_asset(
      &material->shader_asset,
      &temp_memory_pool,
      "skysphere", shaders::Type::standard,
      "skysphere.vert", "skysphere.frag", ""
    );
  }

  // We've created all internal materials, so we will mark the next position
  // in the array of materials, so we know where non-internal materials start.
  engine_state->first_non_internal_material_idx = materials_state->materials.length;

  memory::destroy_memory_pool(&temp_memory_pool);
}


void internals::create_internal_entities(
  EngineState *engine_state,
  renderer::State *renderer_state,
  mats::State *materials_state
) {
  MemoryPool temp_memory_pool = {};

  shaders::init_shader_asset(
    &renderer_state->standard_depth_shader_asset,
    &temp_memory_pool,
    "standard_depth", shaders::Type::depth,
    "standard_depth.vert", "standard_depth.frag",
    "standard_depth.geom"
  );

  // Lighting screenquad
  {
    Entity *entity = entities::add_entity_to_set(
      &engine_state->entity_set, "screenquad_lighting"
    );
    models::ModelLoader *model_loader = engine_state->model_loaders.push();
    models::EntityLoader *entity_loader =
      engine_state->entity_loader_set.loaders[entity->handle];
    models::init_model_loader(model_loader, "builtin:screenquad_lighting");
    models::init_entity_loader(
      entity_loader,
      "screenquad_lighting",
      "builtin:screenquad_lighting",
      drawable::Pass::lighting,
      entity->handle
    );
    models::add_material_to_model_loader(model_loader, "lighting");
  }

  #if USE_BLOOM
    // Preblur screenquad
    {
      Entity *entity = entities::add_entity_to_set(
        &engine_state->entity_set, "screenquad_preblur"
      );
      models::ModelLoader *model_loader = engine_state->model_loaders.push();
      models::EntityLoader *entity_loader =
        engine_state->entity_loader_set.loaders[entity->handle];
      models::init_model_loader(model_loader, "builtin:screenquad_preblur");
      models::init_entity_loader(
        entity_loader,
        "screenquad_preblur",
        "builtin:screenquad_preblur",
        drawable::Pass::preblur,
        entity->handle
      );
      models::add_material_to_model_loader(model_loader, "preblur");
    }

    // Blur 1 screenquad
    {
      Entity *entity = entities::add_entity_to_set(
        &engine_state->entity_set, "screenquad_blur1"
      );
      models::ModelLoader *model_loader = engine_state->model_loaders.push();
      models::EntityLoader *entity_loader =
        engine_state->entity_loader_set.loaders[entity->handle];
      models::init_model_loader(model_loader, "builtin:screenquad_blur1");
      models::init_entity_loader(
        entity_loader,
        "screenquad_blur1",
        "builtin:screenquad_blur1",
        drawable::Pass::blur1,
        entity->handle
      );
      models::add_material_to_model_loader(model_loader, "blur1");
    }

    // Blur 2 screenquad
    {
      Entity *entity = entities::add_entity_to_set(
        &engine_state->entity_set, "screenquad_blur2"
      );
      models::ModelLoader *model_loader = engine_state->model_loaders.push();
      models::EntityLoader *entity_loader =
        engine_state->entity_loader_set.loaders[entity->handle];
      models::init_model_loader(model_loader, "builtin:screenquad_blur2");
      models::init_entity_loader(
        entity_loader,
        "screenquad_blur2",
        "builtin:screenquad_blur2",
        drawable::Pass::blur2,
        entity->handle
      );
      models::add_material_to_model_loader(model_loader, "blur2");
    }
  #endif

  // Postprocessing screenquad
  {
    Entity *entity = entities::add_entity_to_set(
      &engine_state->entity_set, "screenquad_postprocessing"
    );
    models::ModelLoader *model_loader = engine_state->model_loaders.push();
    models::EntityLoader *entity_loader = 
      engine_state->entity_loader_set.loaders[entity->handle];
    models::init_model_loader(model_loader, "builtin:screenquad_postprocessing");
    models::init_entity_loader(
      entity_loader,
      "screenquad_postprocessing",
      "builtin:screenquad_postprocessing",
      drawable::Pass::postprocessing,
      entity->handle
    );
    models::add_material_to_model_loader(model_loader, "postprocessing");
  }

  // Debug screenquad
  {
    Entity *entity = entities::add_entity_to_set(
      &engine_state->entity_set, "screenquad_renderdebug"
    );
    models::ModelLoader *model_loader = engine_state->model_loaders.push();
    models::EntityLoader *entity_loader = 
      engine_state->entity_loader_set.loaders[entity->handle];
    models::init_model_loader(model_loader, "builtin:screenquad_renderdebug");
    models::init_entity_loader(
      entity_loader,
      "screenquad_renderdebug",
      "builtin:screenquad_renderdebug",
      drawable::Pass::renderdebug,
      entity->handle
    );
    models::add_material_to_model_loader(model_loader, "renderdebug");
  }

  // Skysphere
  {
    Entity *entity = entities::add_entity_to_set(
      &engine_state->entity_set, "skysphere"
    );
    models::ModelLoader *model_loader = engine_state->model_loaders.push();
    models::EntityLoader *entity_loader =
      engine_state->entity_loader_set.loaders[entity->handle];
    models::init_model_loader(model_loader, "builtin:skysphere");
    models::init_entity_loader(
      entity_loader,
      "skysphere",
      "builtin:skysphere",
      drawable::Pass::forward_skybox,
      entity->handle
    );
    entity_loader->spatial_component = {
      .entity_handle = entity->handle,
      .position = v3(0.0f),
      .rotation = glm::angleAxis(radians(0.0f), v3(1.0f, 0.0f, 0.0f)),
      .scale = v3(75.0f),
    };
    models::add_material_to_model_loader(model_loader, "skysphere");
  }

  // We've created all internal entities, so we will mark the next position
  // in the EntitySet, to know that that position is where the non-internal
  // entities start.
  engine_state->entity_set.first_non_internal_handle =
    engine_state->entity_set.next_handle;

  memory::destroy_memory_pool(&temp_memory_pool);
}


void internals::init(
  EngineState *engine_state,
  renderer::State *renderer_state,
  mats::State *materials_state
) {
  create_internal_materials(
    engine_state,
    renderer_state,
    materials_state
  );
  create_internal_entities(
    engine_state,
    renderer_state,
    materials_state
  );
}
