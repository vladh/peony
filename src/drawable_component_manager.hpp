#ifndef DRAWABLE_COMPONENT_MANAGER_H
#define DRAWABLE_COMPONENT_MANAGER_H

class SpatialComponentManager;

class DrawableComponentManager {
public:
  DrawableComponentManager(
    Array<DrawableComponent> *components
  );

  DrawableComponent* add(
    EntityHandle entity_handle,
    ModelAsset *model_asset,
    RenderPass target_render_pass
  );

  DrawableComponent* get(EntityHandle handle);

  void draw_all(
    SpatialComponentManager spatial_component_manager,
    RenderPass render_pass, RenderMode render_mode,
    ShaderAsset *entity_depth_shader_asset
  );

private:
  // TODO: std::unordered_map is #slow, find a better implementation.
  // https://stackoverflow.com/questions/3300525/super-high-performance-c-c-hash-map-table-dictionary
  std::unordered_map<EntityHandle, DrawableComponent*> entity_handle_map;

  Array<DrawableComponent> *components;
};

#endif
