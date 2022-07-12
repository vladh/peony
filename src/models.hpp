// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "stackarray.hpp"
#include "mats.hpp"
#include "entities.hpp"
#include "spatial.hpp"
#include "anim.hpp"
#include "behavior.hpp"
#include "physics.hpp"
#include "queue.hpp"
#include "tasks.hpp"
#include "pack.hpp"
#include "lights.hpp"
#include "geom.hpp"
#include "drawable.hpp"

class models {
public:
    // NOTE: Should be at least peony_parser::MAX_N_ARRAY_VALUES
    static constexpr uint32 const MAX_N_COMMON_ARRAY_VALUES = 8;

    enum class ModelLoaderState {
        empty,
        initialized,
        mesh_data_being_loaded,
        mesh_data_loaded,
        vertex_buffers_set_up,
        complete
    };

    struct ModelLoader {
        // These from from the file
        char model_path[MAX_PATH];
        char material_names[MAX_COMMON_NAME_LENGTH][MAX_N_COMMON_ARRAY_VALUES];
        uint32 n_material_names;

        // These are created later
        geom::Mesh meshes[MAX_N_MESHES];
        uint32 n_meshes;
        AnimationComponent animation_component;
        ModelLoaderState state;
    };

    enum class EntityLoaderState {
        empty,
        initialized,
        complete
    };

    struct EntityLoader {
        char name[MAX_DEBUG_NAME_LENGTH];
        char model_path[MAX_PATH];
        EntityHandle entity_handle;
        drawable::Pass render_pass;
        EntityLoaderState state;
        SpatialComponent spatial_component;
        LightComponent light_component;
        BehaviorComponent behavior_component;
        physics::Component physics_component;
    };

    struct EntityLoaderSet {
        Array<EntityLoader> loaders;
    };


#include "models_data.hpp"

    static bool32 prepare_model_loader_and_check_if_done(
        ModelLoader *model_loader,
        Queue<Task> *task_queue,
        BoneMatrixPool *bone_matrix_pool
    );
    static bool32 prepare_entity_loader_and_check_if_done(
        EntityLoader *entity_loader,
        EntitySet *entity_set,
        ModelLoader *model_loader,
        drawable::ComponentSet *drawable_component_set,
        SpatialComponentSet *spatial_component_set,
        LightComponentSet *light_component_set,
        BehaviorComponentSet *behavior_component_set,
        AnimationComponentSet *animation_component_set,
        physics::ComponentSet *physics_component_set
    );
    static bool32 is_model_loader_valid(ModelLoader *model_loader);
    static bool32 is_entity_loader_valid(EntityLoader *entity_loader);
    static void add_material_to_model_loader(
        ModelLoader *model_loader,
        char const *material_name
    );
    static ModelLoader * init_model_loader(
        ModelLoader *model_loader,
        char const *model_path
    );
    static EntityLoader * init_entity_loader(
        EntityLoader *entity_loader,
        const char *name,
        const char *model_path,
        drawable::Pass render_pass,
        EntityHandle entity_handle
    );

private:
    static bool32 is_bone_only_node(aiNode *node);
    static aiNode * find_root_bone(const aiScene *scene);
    static void add_bone_tree_to_animation_component(
        AnimationComponent *animation_component,
        aiNode *node,
        uint32 idx_parent
    );
    static void load_bones(
        AnimationComponent *animation_component,
        const aiScene *scene
    );
    static void load_animations(
        AnimationComponent *animation_component,
        const aiScene *scene,
        BoneMatrixPool *bone_matrix_pool
    );
    static void load_mesh(
        geom::Mesh *mesh,
        aiMesh *ai_mesh,
        const aiScene *scene,
        ModelLoader *model_loader,
        m4 transform,
        Pack indices_pack
    );
    static void load_node(
        ModelLoader *model_loader,
        aiNode *node, const aiScene *scene,
        m4 accumulated_transform, Pack indices_pack
    );
    static void load_model_from_file(ModelLoader *model_loader, BoneMatrixPool *bone_matrix_pool);
    static void load_model_from_data(ModelLoader *model_loader);
};
