glm::mat4 SpatialComponent::make_model_matrix() {
  glm::mat4 model_matrix;
  if (this->parent) {
    model_matrix = this->parent->make_model_matrix();
  } else {
    model_matrix = glm::mat4(1.0f);
  }
  // TODO: This is somehow really #slow, the multiplication in particular.
  // Is there a better way?
  model_matrix = glm::translate(model_matrix, this->position);
  model_matrix = glm::scale(model_matrix, this->scale);
  model_matrix = model_matrix * glm::toMat4(this->rotation);
  return model_matrix;
}
