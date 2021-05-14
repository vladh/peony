namespace spatial {
  // -----------------------------------------------------------
  // Types
  // -----------------------------------------------------------
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


  // -----------------------------------------------------------
  // Private functions
  // -----------------------------------------------------------
  void print_spatial_component(
    SpatialComponent *spatial_component
  ) {
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


  bool32 does_spatial_component_have_dimensions(
    SpatialComponent *spatial_component
  ) {
    return (
      spatial_component->scale.x > 0.0f &&
      spatial_component->scale.y > 0.0f &&
      spatial_component->scale.z > 0.0f
    );
  }


  bool32 is_spatial_component_valid(
    SpatialComponent *spatial_component
  ) {
    return does_spatial_component_have_dimensions(spatial_component) ||
      spatial_component->parent_entity_handle != Entity::no_entity_handle;
  }


  m4 make_model_matrix(
    SpatialComponentSet *spatial_component_set,
    SpatialComponent *spatial_component,
    ModelMatrixCache *cache
  ) {
    m4 model_matrix = m4(1.0f);

    if (spatial_component->parent_entity_handle != Entity::no_entity_handle) {
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


  // -----------------------------------------------------------
  // Public functions
  // -----------------------------------------------------------
}

using spatial::Obb, spatial::Face, spatial::Ray, spatial::SpatialComponent,
  spatial::ModelMatrixCache, spatial::SpatialComponentSet;
