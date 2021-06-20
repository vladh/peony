/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#include "spatial.hpp"
#include "logs.hpp"


void spatial::print_spatial_component(SpatialComponent *spatial_component) {
  logs::info("SpatialComponent:");
  logs::info("  entity_handle: %d", spatial_component->entity_handle);
  logs::info("  position:");
  logs::print_v3(&spatial_component->position);
  logs::info("  rotation:");
  logs::info("(don't know how to print rotation, sorry)");
  /* logs::print_v4(&spatial_component->rotation); */
  logs::info("  scale:");
  logs::print_v3(&spatial_component->scale);
  logs::info("  parent_entity_handle: %d", spatial_component->parent_entity_handle);
}


bool32 spatial::does_spatial_component_have_dimensions(
  SpatialComponent *spatial_component
) {
  return (
    spatial_component->scale.x > 0.0f &&
    spatial_component->scale.y > 0.0f &&
    spatial_component->scale.z > 0.0f
  );
}


bool32 spatial::is_spatial_component_valid(SpatialComponent *spatial_component) {
  return does_spatial_component_have_dimensions(spatial_component) ||
    spatial_component->parent_entity_handle != entities::NO_ENTITY_HANDLE;
}


m4 spatial::make_model_matrix(
  SpatialComponentSet *spatial_component_set,
  SpatialComponent *spatial_component,
  ModelMatrixCache *cache
) {
  m4 model_matrix = m4(1.0f);

  if (spatial_component->parent_entity_handle != entities::NO_ENTITY_HANDLE) {
    SpatialComponent *parent = spatial_component_set->components[
      spatial_component->parent_entity_handle
    ];
    model_matrix = make_model_matrix(spatial_component_set, parent, cache);
  }

  if (does_spatial_component_have_dimensions(spatial_component)) {
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
        glm::toMat4(normalize(spatial_component->rotation));
      cache->last_model_matrix = model_matrix;
      cache->last_model_matrix_spatial_component = spatial_component;
    }
  }

  return model_matrix;
}
