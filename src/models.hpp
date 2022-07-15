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
    static constexpr u32 const MAX_N_COMMON_ARRAY_VALUES = 8;

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
        u32 n_material_names;

        // These are created later
        geom::Mesh meshes[MAX_N_MESHES];
        u32 n_meshes;
        anim::Component animation_component;
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
        entities::Handle entity_handle;
        drawable::Pass render_pass;
        EntityLoaderState state;
        spatial::Component spatial_component;
        lights::Component light_component;
        behavior::Component behavior_component;
        physics::Component physics_component;
    };

#include "models_data.hpp"

    static bool prepare_model_loader_and_check_if_done(ModelLoader *model_loader);
    static bool prepare_entity_loader_and_check_if_done(
        EntityLoader *entity_loader,
        ModelLoader *model_loader
    );
    static bool is_model_loader_valid(ModelLoader *model_loader);
    static bool is_entity_loader_valid(EntityLoader *entity_loader);
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
        entities::Handle entity_handle
    );

private:
    static bool is_bone_only_node(aiNode *node);
    static aiNode * find_root_bone(const aiScene *scene);
    static void add_bone_tree_to_animation_component(
        anim::Component *animation_component,
        aiNode *node,
        u32 idx_parent
    );
    static void load_bones(
        anim::Component *animation_component,
        const aiScene *scene
    );
    static void load_animations(
        anim::Component *animation_component,
        const aiScene *scene
    );
    static void load_mesh(
        geom::Mesh *mesh,
        aiMesh *ai_mesh,
        const aiScene *scene,
        ModelLoader *model_loader,
        m4 transform,
        pack::Pack indices_pack
    );
    static void load_node(
        ModelLoader *model_loader,
        aiNode *node, const aiScene *scene,
        m4 accumulated_transform, pack::Pack indices_pack
    );
    static void load_model_from_file(ModelLoader *model_loader);
    static void load_model_from_data(ModelLoader *model_loader);
};
