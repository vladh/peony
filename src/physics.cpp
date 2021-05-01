bool32 Physics::is_obb_valid(Obb *obb) {
  return obb->extents.x > 0;
}


Obb Physics::apply_model_matrix_to_obb(Obb obb, glm::mat4 *model_matrix) {
  obb.center = glm::vec3(*model_matrix * glm::vec4(obb.center, 1.0f));
  obb.axes[0] = glm::normalize(glm::mat3(*model_matrix) * obb.axes[0]);
  obb.axes[1] = glm::normalize(glm::mat3(*model_matrix) * obb.axes[1]);
  return obb;
}
