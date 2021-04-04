#ifndef ENTITIES_HPP
#define ENTITIES_HPP

struct Mesh;

// NOTE: 0 is an invalid handle.
typedef uint32 EntityHandle;

struct Entity {
  EntityHandle handle;
  char debug_name[MAX_DEBUG_NAME_LENGTH];
  static EntityHandle no_entity_handle;
};

EntityHandle Entity::no_entity_handle = 0;

enum class Behavior {
  none, test
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
  glm::mat4 offset;
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
  glm::mat4 bone_matrices[MAX_N_BONES];
  uint32 n_bones;
  Animation animations[MAX_N_ANIMATIONS];
  uint32 n_animations;
};

enum class LightType {none, point, directional};

struct LightComponent {
  EntityHandle entity_handle;
  LightType type = LightType::none;
  glm::vec3 direction = glm::vec3(0.0f);
  glm::vec4 color = glm::vec4(0.0f);
  glm::vec4 attenuation = glm::vec4(0.0f);
};

struct SpatialComponent {
  EntityHandle entity_handle;
  glm::vec3 position = glm::vec3(0.0f);
  glm::quat rotation = glm::angleAxis(0.0f, glm::vec3(0.0f));
  glm::vec3 scale = glm::vec3(0.0f);
  EntityHandle parent_entity_handle;
};

namespace Entities {
  const char* behavior_to_string(Behavior behavior);
  Behavior behavior_from_string(const char *str);
  bool32 is_behavior_component_valid(
    BehaviorComponent *behavior_component
  );

  bool32 is_animation_component_valid(
    AnimationComponent *animation_component
  );

  const char* light_type_to_string(LightType light_type);
  LightType light_type_from_string(const char *str);
  uint32 light_type_to_int(LightType light_type);
  bool32 is_light_component_valid(
    LightComponent *light_component
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

  bool32 is_entity_loader_valid(EntityLoader *entity_loader);
}

#endif
