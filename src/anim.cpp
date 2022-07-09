// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "util.hpp"
#include "logs.hpp"
#include "engine.hpp"
#include "anim.hpp"
#include "intrinsics.hpp"


namespace anim {
  pny_internal real64* get_bone_matrix_time(
    BoneMatrixPool *pool,
    uint32 idx,
    uint32 idx_bone,
    uint32 idx_anim_key
  ) {
    return pool->times[
      idx * MAX_N_ANIM_KEYS * MAX_N_BONES +
      idx_anim_key * MAX_N_BONES +
      idx_bone
    ];
  }


  pny_internal uint32 get_bone_matrix_anim_key_for_timepoint(
    BoneMatrixPool *bone_matrix_pool,
    AnimationComponent *animation_component,
    real64 animation_timepoint,
    uint32 idx_bone_matrix_set,
    uint32 idx_bone
  ) {
    Bone *bone = &animation_component->bones[idx_bone];
    assert(bone->n_anim_keys > 1);
    uint32 idx_anim_key = bone->last_anim_key;
    do {
      real64 t0 = *get_bone_matrix_time(
        bone_matrix_pool, idx_bone_matrix_set, idx_bone, idx_anim_key
      );
      real64 t1 = *get_bone_matrix_time(
        bone_matrix_pool, idx_bone_matrix_set, idx_bone, idx_anim_key + 1
      );
      if (animation_timepoint > t0 && animation_timepoint < t1) {
        bone->last_anim_key = idx_anim_key;
        return idx_anim_key;
      }
      idx_anim_key++;
      assert(idx_anim_key < bone->n_anim_keys);
      if (idx_anim_key == bone->n_anim_keys - 1) {
        idx_anim_key = 0;
      }
    } while (idx_anim_key != bone->last_anim_key);
    logs::fatal("Could not find anim key.");
    return 0;
  }
}


bool32 anim::is_animation_component_valid(AnimationComponent *animation_component) {
  return animation_component->n_bones > 0 &&
    animation_component->n_animations > 0;
}


uint32 anim::push_to_bone_matrix_pool(BoneMatrixPool *pool) {
  return pool->n_bone_matrix_sets++;
}


m4* anim::get_bone_matrix(
  BoneMatrixPool *pool,
  uint32 idx,
  uint32 idx_bone,
  uint32 idx_anim_key
) {
  return pool->bone_matrices[
    idx * MAX_N_ANIM_KEYS * MAX_N_BONES +
    idx_anim_key * MAX_N_BONES +
    idx_bone
  ];
}


void anim::update_animation_components(
  AnimationComponentSet *animation_component_set,
  SpatialComponentSet *spatial_component_set,
  BoneMatrixPool *bone_matrix_pool
) {
  each (animation_component, animation_component_set->components) {
    if (!is_animation_component_valid(animation_component)) {
      continue;
    }

    Animation *animation = &animation_component->animations[0];

    range_named (idx_bone, 0, animation_component->n_bones) {
      Bone *bone = &animation_component->bones[idx_bone];

      // If we have no anim keys, just return the identity matrix.
      if (bone->n_anim_keys == 0) {
        animation_component->bone_matrices[idx_bone] = m4(1.0f);

      // If we only have one anim key, just return that.
      } else if (bone->n_anim_keys == 1) {
        animation_component->bone_matrices[idx_bone] = *get_bone_matrix(
          bone_matrix_pool,
          animation->idx_bone_matrix_set,
          idx_bone,
          0
        );

      // If we have multiple anim keys, find the right ones and interpolate.
      } else {
        real64 animation_timepoint = fmod(*engine::g_t, animation->duration);

        uint32 idx_anim_key = get_bone_matrix_anim_key_for_timepoint(
          bone_matrix_pool,
          animation_component,
          animation_timepoint,
          animation->idx_bone_matrix_set,
          idx_bone
        );

        real64 t0 = *get_bone_matrix_time(
          bone_matrix_pool, animation->idx_bone_matrix_set, idx_bone, idx_anim_key
        );
        m4 transform_t0 = *get_bone_matrix(
          bone_matrix_pool, animation->idx_bone_matrix_set, idx_bone, idx_anim_key
        );

        real64 t1 = *get_bone_matrix_time(
          bone_matrix_pool, animation->idx_bone_matrix_set, idx_bone, idx_anim_key + 1
        );
        m4 transform_t1 = *get_bone_matrix(
          bone_matrix_pool, animation->idx_bone_matrix_set, idx_bone, idx_anim_key + 1
        );

        real32 lerp_factor = (real32)((animation_timepoint - t0) / (t1 - t0));

        // NOTE: This is probably bad if we have scaling in our transform?
        m4 interpolated_matrix =
          (transform_t0 * (1.0f - lerp_factor)) + (transform_t1 * lerp_factor);

        animation_component->bone_matrices[idx_bone] = interpolated_matrix;
      }
    }
  }
}


void anim::make_bone_matrices_for_animation_bone(
  AnimationComponent *animation_component,
  aiNodeAnim *ai_channel,
  uint32 idx_animation,
  uint32 idx_bone,
  BoneMatrixPool *bone_matrix_pool
) {
  assert(ai_channel->mNumPositionKeys == ai_channel->mNumRotationKeys);
  assert(ai_channel->mNumPositionKeys == ai_channel->mNumScalingKeys);

  Bone *bone = &animation_component->bones[idx_bone];
  bone->n_anim_keys = ai_channel->mNumPositionKeys;

  range_named (idx_anim_key, 0, bone->n_anim_keys) {
    assert(
      ai_channel->mPositionKeys[idx_anim_key].mTime ==
        ai_channel->mRotationKeys[idx_anim_key].mTime
    );
    assert(
      ai_channel->mPositionKeys[idx_anim_key].mTime ==
        ai_channel->mScalingKeys[idx_anim_key].mTime
    );
    real64 anim_key_time = ai_channel->mPositionKeys[idx_anim_key].mTime;

    m4 *bone_matrix = get_bone_matrix(
      bone_matrix_pool,
      animation_component->animations[idx_animation].idx_bone_matrix_set,
      idx_bone,
      idx_anim_key
    );

    real64 *time = get_bone_matrix_time(
      bone_matrix_pool,
      animation_component->animations[idx_animation].idx_bone_matrix_set,
      idx_bone,
      idx_anim_key
    );

    m4 parent_transform = m4(1.0f);

    if (idx_bone > 0) {
      parent_transform = *get_bone_matrix(
        bone_matrix_pool,
        animation_component->animations[idx_animation].idx_bone_matrix_set,
        bone->idx_parent,
        idx_anim_key
      );
    }

    m4 translation = glm::translate(
      m4(1.0f),
      util::aiVector3D_to_glm(&ai_channel->mPositionKeys[idx_anim_key].mValue)
    );
    m4 rotation = glm::toMat4(normalize(
      util::aiQuaternion_to_glm(&ai_channel->mRotationKeys[idx_anim_key].mValue)
    ));
    m4 scaling = glm::scale(
      m4(1.0f),
      util::aiVector3D_to_glm(&ai_channel->mScalingKeys[idx_anim_key].mValue)
    );

    m4 anim_transform = translation * rotation * scaling;
    *bone_matrix = parent_transform * anim_transform;
    *time = anim_key_time;
  }
}


AnimationComponent* anim::find_animation_component(
  SpatialComponent *spatial_component,
  SpatialComponentSet *spatial_component_set,
  AnimationComponentSet *animation_component_set
) {
  AnimationComponent *animation_component =
    animation_component_set->components[spatial_component->entity_handle];

  if (is_animation_component_valid(animation_component)) {
    return animation_component;
  }

  if (spatial_component->parent_entity_handle != entities::NO_ENTITY_HANDLE) {
    SpatialComponent *parent =
      spatial_component_set->components[spatial_component->parent_entity_handle];
    return find_animation_component(
      parent,
      spatial_component_set,
      animation_component_set
    );
  }

  return nullptr;
}


void anim::init(AnimState *anim_state, MemoryPool *pool) {
  anim_state->bone_matrix_pool.bone_matrices = Array<m4>(
    pool,
    MAX_N_ANIMATED_MODELS * MAX_N_BONES * MAX_N_ANIMATIONS * MAX_N_ANIM_KEYS,
    "bone_matrices",
    true
  );
  anim_state->bone_matrix_pool.times = Array<real64>(
    pool,
    MAX_N_ANIMATED_MODELS * MAX_N_BONES * MAX_N_ANIMATIONS * MAX_N_ANIM_KEYS,
    "bone_matrix_times",
    true
  );
}
