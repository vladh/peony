SpatialComponent* SpatialComponentManager::add(
  EntityHandle entity_handle,
  glm::vec3 position,
  glm::quat rotation,
  glm::vec3 scale,
  SpatialComponent *parent
) {
  SpatialComponent *new_component = this->components->push();
  new_component->entity_handle = entity_handle;
  new_component->position = position;
  new_component->rotation = rotation;
  new_component->scale = scale;
  new_component->parent = parent;
  return new_component;
}


SpatialComponent* SpatialComponentManager::add(
  EntityHandle entity_handle,
  glm::vec3 position,
  glm::quat rotation,
  glm::vec3 scale
) {
  SpatialComponent *new_component = this->components->get(entity_handle);
  new_component->entity_handle = entity_handle;
  new_component->position = position;
  new_component->rotation = rotation;
  new_component->scale = scale;
  new_component->parent = nullptr;
  return new_component;
}


SpatialComponent* SpatialComponentManager::get(EntityHandle handle) {
  return this->components->get(handle);
}


glm::mat4 SpatialComponentManager::make_model_matrix(SpatialComponent *spatial_component) {
  glm::mat4 model_matrix = glm::mat4(1.0f);

  if (spatial_component->parent) {
    model_matrix = make_model_matrix(spatial_component->parent);
  }

  if (spatial_component->scale.x > 0.0f) {
    // TODO: This is somehow really #slow, the multiplication in particular.
    // Is there a better way?
    model_matrix = glm::translate(model_matrix, spatial_component->position);
    model_matrix = glm::scale(model_matrix, spatial_component->scale);
    model_matrix = model_matrix * glm::toMat4(spatial_component->rotation);
  }

  return model_matrix;
}


SpatialComponentManager::SpatialComponentManager(
  Array<SpatialComponent> *new_components
) {
  this->components = new_components;
}
