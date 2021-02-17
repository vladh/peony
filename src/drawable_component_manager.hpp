#ifndef DRAWABLE_COMPONENT_MANAGER_HPP
#define DRAWABLE_COMPONENT_MANAGER_HPP

class SpatialComponentManager;
namespace Models {
  struct Mesh;
};

class DrawableComponentManager {
public:
  Array<Entities::DrawableComponent> *components;
  static uint32 last_drawn_shader_program;

  Entities::DrawableComponent* add(
    Entities::DrawableComponent drawable_component
  );
  Entities::DrawableComponent* add(
    Entities::EntityHandle entity_handle,
    Models::Mesh *mesh,
    Renderer::RenderPassFlag target_render_pass
  );
  Entities::DrawableComponent* get(Entities::EntityHandle handle);
  void draw_all(
    SpatialComponentManager *spatial_component_manager,
    Renderer::RenderPassFlag render_pass,
    Renderer::RenderMode render_mode,
    Shaders::ShaderAsset *standard_depth_shader_asset
  );
  void draw(
    Models::Mesh *mesh,
    glm::mat4 *model_matrix,
    glm::mat3 *model_normal_matrix
  );
  void draw_in_depth_mode(
    Models::Mesh *mesh,
    glm::mat4 *model_matrix,
    glm::mat3 *model_normal_matrix,
    Shaders::ShaderAsset *standard_depth_shader_asset
  );
  DrawableComponentManager(
    Array<Entities::DrawableComponent> *components
  );
};

#endif
