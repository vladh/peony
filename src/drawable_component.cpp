bool32 DrawableComponent::is_valid() {
  return this->mesh != nullptr;
}

DrawableComponent::DrawableComponent() {};

DrawableComponent::DrawableComponent(
  EntityHandle entity_handle,
  Mesh *mesh,
  RenderPass::Flag target_render_pass
) :
  entity_handle(entity_handle),
  mesh(mesh),
  target_render_pass(target_render_pass)
{
}
