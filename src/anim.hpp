// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include "types.hpp"
#include "array.hpp"
#include "entities.hpp"
#include "spatial.hpp"

class anim {
public:
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

    struct Component {
        entities::Handle entity_handle;
        Bone bones[MAX_N_BONES];
        m4 bone_matrices[MAX_N_BONES];
        uint32 n_bones;
        Animation animations[MAX_N_ANIMATIONS];
        uint32 n_animations;
    };

    struct ComponentSet {
        Array<Component> components;
    };

    struct State {
        BoneMatrixPool bone_matrix_pool;
    };

    static bool32 is_animation_component_valid(Component *animation_component);
    static uint32 push_to_bone_matrix_pool();
    static m4 * get_bone_matrix(
        uint32 idx,
        uint32 idx_bone,
        uint32 idx_anim_key
    );
    static void update_animation_components(
        ComponentSet *animation_component_set,
        spatial::ComponentSet *spatial_component_set
    );
    static void make_bone_matrices_for_animation_bone(
        Component *animation_component,
        aiNodeAnim *ai_channel,
        uint32 idx_animation,
        uint32 idx_bone
    );
    static Component * find_animation_component(spatial::Component *spatial_component);
    static void init(anim::State *anim_state, MemoryPool *pool);

private:
    static real64 * get_bone_matrix_time(
        uint32 idx,
        uint32 idx_bone,
        uint32 idx_anim_key
    );
    static uint32 get_bone_matrix_anim_key_for_timepoint(
        Component *animation_component,
        real64 animation_timepoint,
        uint32 idx_bone_matrix_set,
        uint32 idx_bone
    );

    static anim::State *state;
};
