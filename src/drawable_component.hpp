#ifndef DRAWABLE_COMPONENT_H
#define DRAWABLE_COMPONENT_H

struct Mesh;

struct DrawableComponent {
  EntityHandle entity_handle = Entity::no_entity_handle;
  Mesh *mesh = nullptr;
  RenderPass::Flag target_render_pass = RenderPass::none;

  bool32 is_valid() {
    return this->mesh != nullptr;
  }

  DrawableComponent() {};

  DrawableComponent(
    EntityHandle entity_handle,
    Mesh *mesh,
    RenderPass::Flag target_render_pass
  ) :
    entity_handle(entity_handle),
    mesh(mesh),
    target_render_pass(target_render_pass)
  {
  }
};

#endif
