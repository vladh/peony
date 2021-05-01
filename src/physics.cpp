bool32 Physics::is_obb_valid(Obb *obb) {
  return obb->extents.x > 0;
}


Obb Physics::transform_obb(Obb obb, SpatialComponent *spatial) {
  glm::mat3 rotation = glm::toMat3(glm::normalize(spatial->rotation));
  obb.center = spatial->position + (rotation * (spatial->scale * obb.center));
  obb.axes[0] = glm::normalize(rotation * obb.axes[0]);
  obb.axes[1] = glm::normalize(rotation * obb.axes[1]);
  obb.extents *= spatial->scale;
  return obb;
}
