#pragma once

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include "types.hpp"
#include "array.hpp"
#include "entities.hpp"
#include "spatial.hpp"

namespace anim {
  struct BoneMatrixPool {
    Array<m4> bone_matrices;
    Array<real64> times;
    uint32 n_bone_matrix_sets;
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

  struct AnimationComponentSet {
    Array<AnimationComponent> components;
  };

  struct AnimState {
    BoneMatrixPool bone_matrix_pool;
  };

  bool32 is_animation_component_valid(AnimationComponent *animation_component);
  uint32 push_to_bone_matrix_pool(BoneMatrixPool *pool);
  m4* get_bone_matrix(
    BoneMatrixPool *pool,
    uint32 idx,
    uint32 idx_bone,
    uint32 idx_anim_key
  );
  void update_animation_components(
    AnimationComponentSet *animation_component_set,
    SpatialComponentSet *spatial_component_set,
    BoneMatrixPool *bone_matrix_pool
  );
  void make_bone_matrices_for_animation_bone(
    AnimationComponent *animation_component,
    aiNodeAnim *ai_channel,
    uint32 idx_animation,
    uint32 idx_bone,
    BoneMatrixPool *bone_matrix_pool
  );
  AnimationComponent* find_animation_component(
    SpatialComponent *spatial_component,
    SpatialComponentSet *spatial_component_set,
    AnimationComponentSet *animation_component_set
  );
  void init(AnimState *anim_state, MemoryPool *pool);
}

using anim::BoneMatrixPool, anim::Bone, anim::Animation,
  anim::AnimationComponent, anim::AnimationComponentSet,
  anim::AnimState;
