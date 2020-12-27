#ifndef DRAWABLE_COMPONENT_HPP
#define DRAWABLE_COMPONENT_HPP

namespace Models {
  struct Mesh;
};

class DrawableComponent {
public:
  EntityHandle entity_handle = Entity::no_entity_handle;
  Models::Mesh *mesh = nullptr;
  Renderer::RenderPassFlag target_render_pass = Renderer::RenderPass::none;

  bool32 is_valid();
  DrawableComponent();
  DrawableComponent(
    EntityHandle entity_handle,
    Models::Mesh *mesh,
    Renderer::RenderPassFlag target_render_pass
  );
};

#endif
