#ifndef ENTITIES_HPP
#define ENTITIES_HPP

struct Mesh;
struct ModelLoader;
struct EntityLoader;

// NOTE: 0 is an invalid handle.
typedef uint32 EntityHandle;

struct Entity {
  EntityHandle handle;
  char debug_name[MAX_DEBUG_NAME_LENGTH];
  static EntityHandle no_entity_handle;
};

EntityHandle Entity::no_entity_handle = 0;

enum class LightType {none, point, directional};

struct LightComponent {
  EntityHandle entity_handle;
  LightType type = LightType::none;
  v3 direction = v3(0.0f);
  v4 color = v4(0.0f);
  v4 attenuation = v4(0.0f);
};

enum class Behavior {
  none,
  test,
  char_movement_test,
  length
};

struct BehaviorComponent {
  EntityHandle entity_handle;
  Behavior behavior = Behavior::none;
};

struct Bone {
  char name[MAX_NODE_NAME_LENGTH];
  uint32 idx_parent;
  uint32 n_anim_keys;
  uint32 last_anim_key;
  m4 offset;
};

struct Animation {
  char name[MAX_NODE_NAME_LENGTH];
  real64 duration;
  // NOTE: Index into the BoneMatrixPool
  uint32 idx_bone_matrix_set;
};

struct AnimationComponent {
  EntityHandle entity_handle;
  Bone bones[MAX_N_BONES];
  m4 bone_matrices[MAX_N_BONES];
  uint32 n_bones;
  Animation animations[MAX_N_ANIMATIONS];
  uint32 n_animations;
};

struct PhysicsComponent {
  EntityHandle entity_handle;
  Obb obb;
  Obb transformed_obb;
};

struct SpatialComponent {
  EntityHandle entity_handle;
  v3 position = v3(0.0f);
  quat rotation = glm::angleAxis(0.0f, v3(0.0f));
  v3 scale = v3(0.0f);
  EntityHandle parent_entity_handle;
};

namespace entities {
  const char* light_type_to_string(LightType light_type);
  LightType light_type_from_string(const char *str);
  uint32 light_type_to_int(LightType light_type);
  bool32 is_light_component_valid(
    LightComponent *light_component
  );

  const char* behavior_to_string(Behavior behavior);
  Behavior behavior_from_string(const char *str);
  bool32 is_behavior_component_valid(
    BehaviorComponent *behavior_component
  );

  bool32 is_animation_component_valid(
    AnimationComponent *animation_component
  );

  bool32 is_physics_component_valid(
    PhysicsComponent *physics_component
  );

  void print_spatial_component(
    SpatialComponent *spatial_component
  );
  bool32 does_spatial_component_have_dimensions(
    SpatialComponent *spatial_component
  );
  bool32 is_spatial_component_valid(
    SpatialComponent *spatial_component
  );

  bool32 is_model_loader_valid(ModelLoader *model_loader);
  bool32 is_entity_loader_valid(EntityLoader *entity_loader);
}

#endif
