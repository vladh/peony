#ifndef DRAWABLE_COMPONENT_H
#define DRAWABLE_COMPONENT_H

struct Mesh;

struct DrawableComponent : public Component {
  Mesh *mesh;
  RenderPass::Flag target_render_pass;
};

#endif
