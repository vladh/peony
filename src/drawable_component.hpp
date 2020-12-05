#ifndef DRAWABLE_COMPONENT_H
#define DRAWABLE_COMPONENT_H

struct DrawableComponent : public Component {
  ModelAsset *model_asset;
  RenderPass::Flag target_render_pass;
};

#endif
