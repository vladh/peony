#ifndef ENTITY_H
#define ENTITY_H

typedef uint32 EntityHandle;

enum EntityType {
  ENTITY_MODEL, ENTITY_SCREENQUAD
};

struct Component {
};

struct DrawableComponent : Component {
  ModelAsset *model_asset;
  RenderPass target_render_pass;
};

struct LightComponent : Component {
  glm::vec4 color;
  glm::vec4 attenuation;
};

struct SpatialComponent : Component {
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 scale;
};

class Entity {
public:
  EntityHandle handle;
  const char *debug_name;
  DrawableComponent *drawable;
  LightComponent *light;
  SpatialComponent *spatial;

  void draw(
    RenderPass render_pass, RenderMode render_mode,
    ShaderAsset *entity_depth_shader_asset
  ) {
    if (!drawable) {
      log_warning("Trying to draw entity %d with no drawable component.", handle);
      return;
    }

    if (render_pass != drawable->target_render_pass) {
      return;
    }

    if (spatial) {
      // TODO: This is somehow really #slow, the multiplication in particular.
      // Is there a better way?
      glm::mat4 model_matrix = glm::mat4(1.0f);
      model_matrix = glm::translate(model_matrix, spatial->position);
      model_matrix = glm::scale(model_matrix, spatial->scale);
      model_matrix = model_matrix * glm::toMat4(spatial->rotation);
      models_draw_model(
        drawable->model_asset, render_mode, &model_matrix, entity_depth_shader_asset
      );
    } else {
      models_draw_model(
        drawable->model_asset, render_mode, nullptr, entity_depth_shader_asset
      );
    }
  }

  void print() {
    log_info("# Entity %d: %s", handle, debug_name);
    if (drawable) {
      log_info("-> Drawable");
      log_info("   model_asset %x", drawable->model_asset);
      log_info("     info.name %s", drawable->model_asset->info.name);
      log_info("     model.meshes.size %d",
        drawable->model_asset->model.meshes.size
      );
      log_info("     model.texture_sets.size %d",
        drawable->model_asset->model.texture_sets.size
      );
      log_info("   target_render_pass %d", drawable->target_render_pass);
    }
    if (light) {
      log_info("-> Light");
      log_info("   color (%f,%f,%f)", light->color.r, light->color.g, light->color.b);
      log_info("   attenuation (%f,%f,%f,%f)",
        light->attenuation[0], light->attenuation[1],
        light->attenuation[2], light->attenuation[3]
      );
    }
    if (spatial) {
      log_info("-> Spatial");
      log_info("   position (%f,%f,%f)",
        spatial->position.x, spatial->position.y, spatial->position.z
      );
      log_info("   rotation (%f %f,%f,%f)",
        spatial->rotation.w, spatial->rotation.x, spatial->rotation.y, spatial->rotation.z
      );
      log_info("   scale (%f,%f,%f)",
        spatial->scale.x, spatial->scale.y, spatial->scale.z
      );
    }
    log_newline();
  }
};

class EntityManager {
public:
  EntityManager(
    Array<Entity> *entities,
    Array<DrawableComponent> *drawable_components,
    Array<LightComponent> *light_components,
    Array<SpatialComponent> *spatial_components
  ) {
    this->entities = entities;
    this->drawable_components = drawable_components;
    this->light_components = light_components;
    this->spatial_components = spatial_components;
  }

  Entity* add(const char* debug_name) {
    Entity *new_entity = array_push<Entity>(entities);
    new_entity->handle = make_handle();
    new_entity->debug_name = debug_name;
    entity_handle_map[new_entity->handle] = new_entity;
    return new_entity;
  }

  void add_drawable_component(
    Entity *entity,
    ModelAsset *model_asset,
    RenderPass target_render_pass
  ) {
    entity->drawable = array_push(drawable_components);
    entity->drawable->model_asset = model_asset;
    entity->drawable->target_render_pass = target_render_pass;
  }

  void add_light_component(
    Entity *entity,
    glm::vec4 color,
    glm::vec4 attenuation
  ) {
    entity->light = array_push<LightComponent>(light_components);
    entity->light->color = color;
    entity->light->attenuation = attenuation;
  }

  void add_spatial_component(
    Entity *entity,
    glm::vec3 position,
    glm::quat rotation,
    glm::vec3 scale
  ) {
    entity->spatial = array_push<SpatialComponent>(spatial_components);
    entity->spatial->position = position;
    entity->spatial->rotation = rotation;
    entity->spatial->scale = scale;
  }

  Entity* get(EntityHandle handle) {
    if (entity_handle_map.count(handle)) {
      return entity_handle_map[handle];
    } else {
      return nullptr;
    }
  }

  void draw_all(
    RenderPass render_pass, RenderMode render_mode,
    ShaderAsset *entity_depth_shader_asset
  ) {
    for (uint32 idx = 0; idx < entities->size; idx++) {
      Entity *entity = &entities->items[idx];
      entity->draw(render_pass, render_mode, entity_depth_shader_asset);
    }
  }

  void print_all() {
    for (uint32 idx = 0; idx < entities->size; idx++) {
      Entity *entity = &entities->items[idx];
      entity->print();
    }
  }

private:
  EntityHandle last_handle = 0;
  // TODO: std::unordered_map is #slow, find a better implementation.
  std::unordered_map<EntityHandle, Entity*> entity_handle_map;
  uint32 n_entities = 0;

  Array<Entity> *entities;
  Array<LightComponent> *light_components;
  Array<SpatialComponent> *spatial_components;
  Array<DrawableComponent> *drawable_components;

  EntityHandle make_handle() {
    return last_handle++;
  }
};

#endif
