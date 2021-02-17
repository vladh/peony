Entities::SpatialComponent* SpatialComponentManager::add(
  Entities::SpatialComponent spatial_component
) {
  assert(spatial_component.entity_handle != Entities::Entity::no_entity_handle);
  Entities::SpatialComponent *new_component = this->components->get(
    spatial_component.entity_handle
  );
  *new_component = spatial_component;
  return new_component;
}


Entities::SpatialComponent* SpatialComponentManager::add(
  Entities::EntityHandle entity_handle,
  glm::vec3 position,
  glm::quat rotation,
  glm::vec3 scale,
  Entities::EntityHandle parent_entity_handle
) {
  assert(entity_handle != Entities::Entity::no_entity_handle);
  Entities::SpatialComponent *new_component = this->components->get(entity_handle);
  new_component->entity_handle = entity_handle;
  new_component->position = position;
  new_component->rotation = rotation;
  new_component->scale = scale;
  new_component->parent_entity_handle = parent_entity_handle;
  return new_component;
}


Entities::SpatialComponent* SpatialComponentManager::add(
  Entities::EntityHandle entity_handle,
  glm::vec3 position,
  glm::quat rotation,
  glm::vec3 scale
) {
  assert(entity_handle != Entities::Entity::no_entity_handle);
  Entities::SpatialComponent *new_component = this->components->get(entity_handle);
  new_component->entity_handle = entity_handle;
  new_component->position = position;
  new_component->rotation = rotation;
  new_component->scale = scale;
  new_component->parent_entity_handle = Entities::Entity::no_entity_handle;
  return new_component;
}


Entities::SpatialComponent* SpatialComponentManager::get(
  Entities::EntityHandle handle
) {
  if (handle == Entities::Entity::no_entity_handle) {
    return nullptr;
  }
  return this->components->get(handle);
}


glm::mat4 SpatialComponentManager::make_model_matrix(
  Entities::SpatialComponent *spatial_component,
  ModelMatrixCache *cache
) {
  glm::mat4 model_matrix = glm::mat4(1.0f);

  if (spatial_component->parent_entity_handle != Entities::Entity::no_entity_handle) {
    Entities::SpatialComponent *parent = get(spatial_component->parent_entity_handle);
    model_matrix = make_model_matrix(parent, cache);
  }

  if (Entities::does_spatial_component_have_dimensions(spatial_component)) {
    // TODO: This is somehow really #slow, the multiplication in particular.
    // Is there a better way?
    if (
      spatial_component == cache->last_model_matrix_spatial_component
    ) {
      model_matrix = cache->last_model_matrix;
    } else {
      model_matrix = glm::translate(model_matrix, spatial_component->position);
      model_matrix = glm::scale(model_matrix, spatial_component->scale);
      model_matrix = model_matrix * glm::toMat4(spatial_component->rotation);
      cache->last_model_matrix = model_matrix;
      cache->last_model_matrix_spatial_component = spatial_component;
    }
  }

  return model_matrix;
}


SpatialComponentManager::SpatialComponentManager(
  Array<Entities::SpatialComponent> *new_components
) {
  this->components = new_components;
}
