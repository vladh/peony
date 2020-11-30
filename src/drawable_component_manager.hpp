#ifndef DRAWABLE_COMPONENT_MANAGER_H
#define DRAWABLE_COMPONENT_MANAGER_H

class SpatialComponentManager;

class DrawableComponentManager {
public:
  Array<DrawableComponent> *components;

  DrawableComponentManager(
    Array<DrawableComponent> *components
  );

  DrawableComponent* add(
    EntityHandle entity_handle,
    ModelAsset *model_asset,
    RenderPass::Flag target_render_pass
  );

  DrawableComponent* get(EntityHandle handle);

  void draw_all(
    Memory *memory,
    PersistentPbo *persistent_pbo,
    TextureNamePool *texture_name_pool,
    SpatialComponentManager *spatial_component_manager,
    Queue<Task> *task_queue,
    RenderPass::Flag render_pass, RenderMode render_mode,
    ShaderAsset *standard_depth_shader_asset
  );

private:
};

#endif
