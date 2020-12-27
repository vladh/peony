bool32 DrawableComponent::is_valid() {
  return this->mesh != nullptr;
}


DrawableComponent::DrawableComponent() {};


DrawableComponent::DrawableComponent(
  EntityHandle entity_handle,
  Models::Mesh *mesh,
  Renderer::RenderPassFlag target_render_pass
) :
  entity_handle(entity_handle),
  mesh(mesh),
  target_render_pass(target_render_pass)
{
}
