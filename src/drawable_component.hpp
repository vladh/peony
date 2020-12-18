#ifndef DRAWABLE_COMPONENT_HPP
#define DRAWABLE_COMPONENT_HPP

struct Mesh;

class DrawableComponent {
public:
  EntityHandle entity_handle = Entity::no_entity_handle;
  Mesh *mesh = nullptr;
  RenderPass::Flag target_render_pass = RenderPass::none;

  bool32 is_valid();
  DrawableComponent();
  DrawableComponent(
    EntityHandle entity_handle,
    Mesh *mesh,
    RenderPass::Flag target_render_pass
  );
};

#endif
