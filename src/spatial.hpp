#pragma once

#include "types.hpp"
#include "entities.hpp"

namespace spatial {
  struct Obb {
    v3 center;
    v3 x_axis;
    v3 y_axis; // We can get the z axis with a cross product
    v3 extents;
  };

  struct Face {
    v3 vertices[4];
  };

  struct Ray {
    v3 origin;
    v3 direction;
  };


  struct SpatialComponent {
    EntityHandle entity_handle;
    v3 position = v3(0.0f);
    quat rotation = glm::angleAxis(0.0f, v3(0.0f));
    v3 scale = v3(0.0f);
    EntityHandle parent_entity_handle;
  };


  struct ModelMatrixCache {
    m4 last_model_matrix;
    SpatialComponent *last_model_matrix_spatial_component;
  };

  struct SpatialComponentSet {
    Array<SpatialComponent> components;
  };


  void print_spatial_component(SpatialComponent *spatial_component);
  bool32 does_spatial_component_have_dimensions(SpatialComponent *spatial_component);
  bool32 is_spatial_component_valid(SpatialComponent *spatial_component);
  m4 make_model_matrix(
    SpatialComponentSet *spatial_component_set,
    SpatialComponent *spatial_component,
    ModelMatrixCache *cache
  );
}

using spatial::Obb, spatial::Face, spatial::Ray, spatial::SpatialComponent,
  spatial::ModelMatrixCache, spatial::SpatialComponentSet;
