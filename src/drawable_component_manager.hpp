#ifndef DRAWABLE_COMPONENT_MANAGER_H
#define DRAWABLE_COMPONENT_MANAGER_H

class SpatialComponentManager;

class DrawableComponentManager {
public:
  Array<DrawableComponent> *components;
  static uint32 last_drawn_shader_program;

  DrawableComponentManager(
    Array<DrawableComponent> *components
  );

  DrawableComponent* add(
    DrawableComponent drawable_component
  );

  DrawableComponent* add(
    EntityHandle entity_handle,
    Mesh *mesh,
    RenderPass::Flag target_render_pass
  );

  DrawableComponent* get(EntityHandle handle);

  void draw_all(
    SpatialComponentManager *spatial_component_manager,
    RenderPass::Flag render_pass,
    RenderMode render_mode,
    ShaderAsset *standard_depth_shader_asset
  );
  void draw(
    Mesh *mesh,
    glm::mat4 *model_matrix,
    glm::mat3 *model_normal_matrix
  );
  void draw_in_depth_mode(
    Mesh *mesh,
    glm::mat4 *model_matrix,
    glm::mat3 *model_normal_matrix,
    ShaderAsset *standard_depth_shader_asset
  );
};

#endif
