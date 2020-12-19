void SpatialComponent::print() {
  log_info("SpatialComponent:");
  log_info("  entity_handle: %d", this->entity_handle);
  log_info("  position:");
  log_vec3(&this->position);
  log_info("  rotation:");
  log_info("(don't know how to print rotation, sorry)");
  /* log_vec4(&this->rotation); */
  log_info("  scale:");
  log_vec3(&this->scale);
  log_info("  parent_entity_handle: %d", this->parent_entity_handle);
}


bool32 SpatialComponent::has_dimensions() {
  return (
    this->scale.x > 0.0f &&
    this->scale.y > 0.0f &&
    this->scale.z > 0.0f
  );
}


bool32 SpatialComponent::is_valid() {
  return has_dimensions() ||
    this->parent_entity_handle != Entity::no_entity_handle;
}


SpatialComponent::SpatialComponent() {};


SpatialComponent::SpatialComponent(
  EntityHandle entity_handle,
  glm::vec3 position,
  glm::quat rotation,
  glm::vec3 scale,
  EntityHandle parent_entity_handle
) :
  entity_handle(entity_handle),
  position(position),
  rotation(rotation),
  scale(scale),
  parent_entity_handle(parent_entity_handle)
{
}


SpatialComponent::SpatialComponent(
  EntityHandle entity_handle,
  glm::vec3 position,
  glm::quat rotation,
  glm::vec3 scale
) :
  entity_handle(entity_handle),
  position(position),
  rotation(rotation),
  scale(scale)
{
}
