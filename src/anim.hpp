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
        Array<f64> times;
        u32 n_bone_matrix_sets;
    };

    struct Bone {
        char name[MAX_NODE_NAME_LENGTH];
        u32 idx_parent;
        u32 n_anim_keys;
        u32 last_anim_key;
        m4 offset;
    };

    struct Animation {
        char name[MAX_NODE_NAME_LENGTH];
        f64 duration;
        // NOTE: Index into the BoneMatrixPool
        u32 idx_bone_matrix_set;
    };

    struct Component {
        entities::Handle entity_handle;
        Bone bones[MAX_N_BONES];
        m4 bone_matrices[MAX_N_BONES];
        u32 n_bones;
        Animation animations[MAX_N_ANIMATIONS];
        u32 n_animations;
    };

    struct ComponentSet {
        Array<Component> components;
    };

    struct State {
        BoneMatrixPool bone_matrix_pool;
    };

    static bool is_animation_component_valid(Component *animation_component);
    static u32 push_to_bone_matrix_pool();
    static m4 * get_bone_matrix(
        u32 idx,
        u32 idx_bone,
        u32 idx_anim_key
    );
    static void update_animation_components(ComponentSet *animation_component_set);
    static void make_bone_matrices_for_animation_bone(
        Component *animation_component,
        aiNodeAnim *ai_channel,
        u32 idx_animation,
        u32 idx_bone
    );
    static Component * find_animation_component(spatial::Component *spatial_component);
    static void init(anim::State *anim_state, memory::Pool *pool);

private:
    static f64 * get_bone_matrix_time(
        u32 idx,
        u32 idx_bone,
        u32 idx_anim_key
    );
    static u32 get_bone_matrix_anim_key_for_timepoint(
        Component *animation_component,
        f64 animation_timepoint,
        u32 idx_bone_matrix_set,
        u32 idx_bone
    );

    static anim::State *state;
};
