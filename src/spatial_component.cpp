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
