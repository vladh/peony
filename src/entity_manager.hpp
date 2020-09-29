#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

class EntityManager {
public:
  EntityManager(
    Array<Entity> *entities,
    Array<DrawableComponent> *drawable_components,
    Array<LightComponent> *light_components,
    Array<SpatialComponent> *spatial_components
  );

  Entity* add(const char* debug_name);

  void add_drawable_component(
    Entity *entity,
    ModelAsset *model_asset,
    RenderPass target_render_pass
  );

  void add_light_component(
    Entity *entity,
    glm::vec4 color,
    glm::vec4 attenuation
  );

  void add_spatial_component(
    Entity *entity,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale
  );

  Entity* get(EntityHandle handle);

  void draw_all(
    RenderPass render_pass, RenderMode render_mode,
    ShaderAsset *entity_depth_shader_asset
  );

  void print_all();

private:
  EntityHandle last_handle = 0;
  // TODO: std::unordered_map is #slow, find a better implementation.
  // https://stackoverflow.com/questions/3300525/super-high-performance-c-c-hash-map-table-dictionary
  std::unordered_map<EntityHandle, Entity*> entity_handle_map;
  uint32 n_entities = 0;

  Array<Entity> *entities;
  Array<LightComponent> *light_components;
  Array<SpatialComponent> *spatial_components;
  Array<DrawableComponent> *drawable_components;

  EntityHandle make_handle();
};

#endif
