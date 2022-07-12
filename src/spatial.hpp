// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

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


  struct Component {
    entities::Handle entity_handle;
    v3 position;
    quat rotation;
    v3 scale;
    entities::Handle parent_entity_handle;
  };


  struct ModelMatrixCache {
    m4 last_model_matrix;
    Component *last_model_matrix_spatial_component;
  };

  struct ComponentSet {
    Array<Component> components;
  };


  void print_spatial_component(Component *spatial_component);
  bool32 does_spatial_component_have_dimensions(Component *spatial_component);
  bool32 is_spatial_component_valid(Component *spatial_component);
  m4 make_model_matrix(
    Component *spatial_component,
    ModelMatrixCache *cache
  );
}

using spatial::Obb, spatial::Face, spatial::Ray, spatial::Component,
  spatial::ModelMatrixCache, spatial::ComponentSet;
