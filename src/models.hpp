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

class models {
public:
    // NOTE: Should be at least peony_parser::MAX_N_ARRAY_VALUES
    static constexpr uint32 const MAX_N_COMMON_ARRAY_VALUES = 8;

    enum class RenderMode { regular, depth };

    enum class RenderPass : uint32 {
        none = 0,
        shadowcaster = (1 << 0),
        deferred = (1 << 1),
        forward_depth = (1 << 2),
        forward_nodepth = (1 << 3),
        forward_skybox = (1 << 4),
        lighting = (1 << 5),
        postprocessing = (1 << 6),
        preblur = (1 << 7),
        blur1 = (1 << 8),
        blur2 = (1 << 9),
        renderdebug = (1 << 10),
    };

    struct Vertex {
        v3 position;
        v3 normal;
        v2 tex_coords;
        uint32 bone_idxs[MAX_N_BONES_PER_VERTEX];
        real32 bone_weights[MAX_N_BONES_PER_VERTEX];
    };

    struct Mesh {
        MemoryPool temp_memory_pool;
        m4 transform;
        char material_name[MAX_COMMON_NAME_LENGTH];
        Pack indices_pack;
        uint32 vao;
        uint32 vbo;
        uint32 ebo;
        GLenum mode;
        Vertex *vertices;
        uint32 *indices;
        uint32 n_vertices;
        uint32 n_indices;
    };

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
        Mesh meshes[MAX_N_MESHES];
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
        RenderPass render_pass;
        EntityLoaderState state;
        SpatialComponent spatial_component;
        LightComponent light_component;
        BehaviorComponent behavior_component;
        physics::Component physics_component;
    };

    struct EntityLoaderSet {
        Array<EntityLoader> loaders;
    };

    struct Component {
        EntityHandle entity_handle;
        Mesh mesh;
        RenderPass target_render_pass = RenderPass::none;
    };

    struct ComponentSet {
        Array<models::Component> components;
        uint32 last_drawn_shader_program;
    };

#include "models_data.hpp"

    static char const * render_pass_to_string(RenderPass render_pass);
    static RenderPass render_pass_from_string(const char* str);
    static bool32 prepare_model_loader_and_check_if_done(
        ModelLoader *model_loader,
        mats::PersistentPbo *persistent_pbo,
        mats::TextureNamePool *texture_name_pool,
        Queue<Task> *task_queue,
        BoneMatrixPool *bone_matrix_pool
    );
    static bool32 prepare_entity_loader_and_check_if_done(
        EntityLoader *entity_loader,
        EntitySet *entity_set,
        ModelLoader *model_loader,
        models::ComponentSet *drawable_component_set,
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
        RenderPass render_pass,
        EntityHandle entity_handle
    );
    static bool32 is_drawable_component_valid(models::Component *drawable_component);
    static void destroy_drawable_component(models::Component *drawable_component);

private:
    static void make_plane(
        MemoryPool *memory_pool,
        uint32 x_size, uint32 z_size,
        uint32 n_x_segments, uint32 n_z_segments,
        uint32 *n_vertices, uint32 *n_indices,
        Vertex **vertex_data, uint32 **index_data
    );
    static void make_sphere(
        MemoryPool *memory_pool,
        uint32 n_x_segments, uint32 n_y_segments,
        uint32 *n_vertices, uint32 *n_indices,
        Vertex **vertex_data, uint32 **index_data
    );
    static void setup_mesh_vertex_buffers(
        Mesh *mesh,
        Vertex *vertex_data, uint32 n_vertices,
        uint32 *index_data, uint32 n_indices
    );
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
        Mesh *mesh,
        aiMesh *ai_mesh,
        const aiScene *scene,
        ModelLoader *model_loader,
        m4 transform,
        Pack indices_pack
    );
    static void destroy_mesh(Mesh *mesh);
    static void load_node(
        ModelLoader *model_loader,
        aiNode *node, const aiScene *scene,
        m4 accumulated_transform, Pack indices_pack
    );
    static void load_model_from_file(ModelLoader *model_loader, BoneMatrixPool *bone_matrix_pool);
    static void load_model_from_data(ModelLoader *model_loader);
    static bool32 is_mesh_valid(Mesh *mesh);
};
