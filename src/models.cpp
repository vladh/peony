// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "../src_external/glad/glad.h"
#include "../src_external/pstr.h"
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include "pack.hpp"
#include "logs.hpp"
#include "models.hpp"
#include "debug.hpp"
#include "util.hpp"
#include "intrinsics.hpp"


bool32
models::prepare_model_loader_and_check_if_done(ModelLoader *model_loader) {
    if (model_loader->state == ModelLoaderState::initialized) {
        if (pstr_starts_with(model_loader->model_path, "builtin:")) {
            logs::error("Found model with builtin model_path for which no vertex data was loaded.");
            return false;
        }
        tasks::push({
            .fn = (tasks::TaskFn)load_model_from_file,
            .argument_1 = (void*)model_loader,
        });
        model_loader->state = ModelLoaderState::mesh_data_being_loaded;
    }

    if (model_loader->state == ModelLoaderState::mesh_data_being_loaded) {
        // Wait. The task will progress this for us.
    }

    if (model_loader->state == ModelLoaderState::mesh_data_loaded) {
        for (uint32 idx = 0; idx < model_loader->n_meshes; idx++) {
            geom::Mesh *mesh = &model_loader->meshes[idx];
            geom::setup_mesh_vertex_buffers(mesh, mesh->vertices, mesh->n_vertices, mesh->indices, mesh->n_indices);
            memory::destroy_memory_pool(&mesh->temp_memory_pool);
        }
        model_loader->state = ModelLoaderState::vertex_buffers_set_up;
    }

    if (model_loader->state == ModelLoaderState::vertex_buffers_set_up) {
        // Set material names for each mesh
        range_named (idx_material, 0, model_loader->n_material_names) {
            range_named (idx_mesh, 0, model_loader->n_meshes) {
                geom::Mesh *mesh = &model_loader->meshes[idx_mesh];
                uint8 mesh_number = pack::get(&mesh->indices_pack, 0);
                // For our model's mesh number `mesh_number`, we want to choose
                // material `idx_mesh` such that `mesh_number == idx_mesh`, i.e.
                // we choose the 4th material for mesh number 4.
                // However, if we have more meshes than materials, the extra
                // meshes all get material number 0.
                if (
                    mesh_number == idx_material ||
                    (mesh_number >= model_loader->n_material_names && idx_material == 0)
                ) {
                    pstr_copy(mesh->material_name, MAX_COMMON_NAME_LENGTH, model_loader->material_names[idx_material]);
                }
            }
        }

        model_loader->state = ModelLoaderState::complete;
    }

    if (model_loader->state == ModelLoaderState::complete) {
        return true;
    }

    return false;
}


bool32
models::prepare_entity_loader_and_check_if_done(
    EntityLoader *entity_loader,
    EntitySet *entity_set,
    ModelLoader *model_loader,
    drawable::ComponentSet *drawable_component_set,
    SpatialComponentSet *spatial_component_set,
    lights::ComponentSet *light_component_set,
    behavior::ComponentSet *behavior_component_set,
    anim::ComponentSet *animation_component_set,
    physics::ComponentSet *physics_component_set
) {
    if (entity_loader->state == EntityLoaderState::initialized) {
        // Before we can create entities, we need this entity's models to have
        // been loaded.
        if (model_loader->state != ModelLoaderState::complete) {
            return false;
        }

        SpatialComponent *spatial_component = spatial_component_set->components[entity_loader->entity_handle];
        *spatial_component = entity_loader->spatial_component;
        spatial_component->entity_handle = entity_loader->entity_handle;

        lights::Component *light_component = light_component_set->components[entity_loader->entity_handle];
        *light_component = entity_loader->light_component;
        light_component->entity_handle = entity_loader->entity_handle;

        behavior::Component *behavior_component = behavior_component_set->components[entity_loader->entity_handle];
        *behavior_component = entity_loader->behavior_component;
        behavior_component->entity_handle = entity_loader->entity_handle;

        anim::Component *animation_component = animation_component_set->components[entity_loader->entity_handle];
        *animation_component = model_loader->animation_component;
        animation_component->entity_handle = entity_loader->entity_handle;

        physics::Component *physics_component = physics_component_set->components[entity_loader->entity_handle];
        *physics_component = entity_loader->physics_component;
        physics_component->entity_handle = entity_loader->entity_handle;

        // drawable::Component
        if (model_loader->n_meshes == 1) {
            drawable::Component *drawable_component = drawable_component_set->components[entity_loader->entity_handle];
            assert(drawable_component);
            *drawable_component = {
                .entity_handle = entity_loader->entity_handle,
                .mesh = model_loader->meshes[0],
                .target_render_pass = entity_loader->render_pass,
            };
        } else if (model_loader->n_meshes > 1) {
            for (uint32 idx = 0; idx < model_loader->n_meshes; idx++) {
                geom::Mesh *mesh = &model_loader->meshes[idx];

                Entity *child_entity = entities::add_entity_to_set(entity_set, entity_loader->name);

                if (spatial::is_spatial_component_valid(&entity_loader->spatial_component)) {
                    SpatialComponent *child_spatial_component = spatial_component_set->components[child_entity->handle];
                    assert(child_spatial_component);
                    *child_spatial_component = {
                        .entity_handle = child_entity->handle,
                        .position = v3(0.0f),
                        .rotation = glm::angleAxis(radians(0.0f), v3(0.0f)),
                        .scale = v3(0.0f),
                        .parent_entity_handle = entity_loader->entity_handle,
                    };
                }

                drawable::Component *drawable_component = drawable_component_set->components[child_entity->handle];
                assert(drawable_component);
                *drawable_component = {
                    .entity_handle = child_entity->handle,
                    .mesh = *mesh,
                    .target_render_pass = entity_loader->render_pass,
                };
            }
        }

        entity_loader->state = EntityLoaderState::complete;
    }

    if (entity_loader->state == EntityLoaderState::complete) {
        return true;
    }

    return false;
}


bool32
models::is_model_loader_valid(ModelLoader *model_loader)
{
    return model_loader->state != ModelLoaderState::empty;
}


bool32
models::is_entity_loader_valid(EntityLoader *entity_loader)
{
    return entity_loader->state != EntityLoaderState::empty;
}


void
models::add_material_to_model_loader(
    ModelLoader *model_loader,
    char const *material_name
) {
    pstr_copy(
        model_loader->material_names[model_loader->n_material_names++],
        MAX_COMMON_NAME_LENGTH,
        material_name
    );
}


models::ModelLoader *
models::init_model_loader(
    ModelLoader *model_loader,
    char const *model_path
) {
    assert(model_loader);
    pstr_copy(model_loader->model_path, MAX_PATH, model_path);

    model_loader->state = ModelLoaderState::initialized;

    if (pstr_starts_with(model_path, "builtin:")) {
        load_model_from_data(model_loader);
    }

    return model_loader;
}


models::EntityLoader *
models::init_entity_loader(
    EntityLoader *entity_loader,
    const char *name,
    const char *model_path,
    drawable::Pass render_pass,
    EntityHandle entity_handle
) {
    assert(entity_loader);
    pstr_copy(entity_loader->name, MAX_COMMON_NAME_LENGTH, name);
    pstr_copy(entity_loader->model_path, MAX_PATH, model_path);
    entity_loader->render_pass = render_pass;
    entity_loader->entity_handle = entity_handle;
    // TODO: Can we move this to constructor?
    // If so, can we do so for other init_*() methods?
    entity_loader->state = EntityLoaderState::initialized;
    return entity_loader;
}


bool32
models::is_bone_only_node(aiNode *node)
{
    if (node->mNumMeshes > 0) {
        return false;
    }
    bool32 have_we_found_it = true;
    range (0, node->mNumChildren) {
        if (!is_bone_only_node(node->mChildren[idx])) {
            have_we_found_it = false;
        }
    }
    return have_we_found_it;
}


aiNode *
models::find_root_bone(const aiScene *scene)
{
    // NOTE: To find the root bone, we find the first-level node (direct child
    // of root node) whose entire descendent tree has no meshes, including the
    // leaf nodes. Is this a perfect way of finding the root bone? Probably
    // not. Is it good enough? Sure looks like it! :)
    aiNode *root_node = scene->mRootNode;

    range (0, root_node->mNumChildren) {
        aiNode *first_level_node = root_node->mChildren[idx];
        if (is_bone_only_node(first_level_node)) {
            return first_level_node;
        }
    }

    return nullptr;
}


void
models::add_bone_tree_to_animation_component(
    anim::Component *animation_component,
    aiNode *node,
    uint32 idx_parent
) {
    uint32 idx_new_bone = animation_component->n_bones;
    animation_component->bones[idx_new_bone] = {
        .idx_parent = idx_parent,
        // NOTE: offset is added later, since we don't have the aiBone at this stage.
    };
    pstr_copy(animation_component->bones[idx_new_bone].name, MAX_NODE_NAME_LENGTH, node->mName.C_Str());
    animation_component->n_bones++;

    range (0, node->mNumChildren) {
        add_bone_tree_to_animation_component(animation_component, node->mChildren[idx], idx_new_bone);
    }
}


void
models::load_bones(
    anim::Component *animation_component,
    const aiScene *scene
) {
    aiNode *root_bone = find_root_bone(scene);

    if (!root_bone) {
        // No bones. Okay!
        return;
    }

    // The root will just have its parent marked as itself, to avoid using
    // a -1 index and so on. This is fine, because the root will always be
    // index 0, so we can just disregard the parent if we're on index 0.
    add_bone_tree_to_animation_component(animation_component, root_bone, 0);
}


void
models::load_animations(
    anim::Component *animation_component,
    const aiScene *scene
) {
    m4 scene_root_transform = util::aimatrix4x4_to_glm(&scene->mRootNode->mTransformation);
    m4 inverse_scene_root_transform = inverse(scene_root_transform);

    animation_component->n_animations = scene->mNumAnimations;
    range_named (idx_animation, 0, scene->mNumAnimations) {
        anim::Animation *animation = &animation_component->animations[idx_animation];
        aiAnimation *ai_animation = scene->mAnimations[idx_animation];

        *animation = {
            .duration = ai_animation->mDuration * ai_animation->mTicksPerSecond,
            .idx_bone_matrix_set = anim::push_to_bone_matrix_pool(),
        };
        pstr_copy(animation->name, MAX_NODE_NAME_LENGTH, ai_animation->mName.C_Str());

        // Calculate bone matrices.
        // NOTE: We do not finalise the bone matrices at this stage!
        // The matrices in local form are still needed for the children.
        range_named(idx_bone, 0, animation_component->n_bones) {
            anim::Bone *bone = &animation_component->bones[idx_bone];

            uint32 found_channel_idx = 0;
            bool32 did_find_channel = false;

            range_named (idx_channel, 0, ai_animation->mNumChannels) {
                aiNodeAnim *ai_channel = ai_animation->mChannels[idx_channel];
                if (pstr_eq(ai_channel->mNodeName.C_Str(), bone->name)) {
                    found_channel_idx = idx_channel;
                    did_find_channel = true;
                    break;
                }
            }

            if (!did_find_channel) {
                // No channel for this bone. Maybe it's just not animated. Skip it.
                continue;
            }

            anim::make_bone_matrices_for_animation_bone(animation_component,
                ai_animation->mChannels[found_channel_idx],
                idx_animation, idx_bone);
        }

        // Finalise bone matrices.
        // NOTE: Now that we've calculated all the bone matrices for this
        // animation, we can finalise them.
        range_named(idx_bone, 0, animation_component->n_bones) {
            anim::Bone *bone = &animation_component->bones[idx_bone];

            range_named (idx_anim_key, 0, bone->n_anim_keys) {
                // #slow: We could avoid this multiplication here.
                m4 *bone_matrix = anim::get_bone_matrix(animation->idx_bone_matrix_set,
                    idx_bone, idx_anim_key);

                *bone_matrix =
                    scene_root_transform *
                    *bone_matrix *
                    bone->offset *
                    inverse_scene_root_transform;
            }
        }
    }
}


void
models::load_mesh(
    geom::Mesh *mesh,
    aiMesh *ai_mesh,
    const aiScene *scene,
    ModelLoader *model_loader,
    m4 transform,
    Pack indices_pack
) {
    mesh->transform = transform;
    m3 normal_matrix = m3(transpose(inverse(transform)));
    mesh->mode = GL_TRIANGLES;

    mesh->indices_pack = indices_pack;

    // Vertices
    if (!ai_mesh->mNormals) {
        logs::warning("Model does not have normals.");
    }

    mesh->n_vertices = ai_mesh->mNumVertices;
    mesh->vertices = (geom::Vertex*)memory::push(&mesh->temp_memory_pool,
        mesh->n_vertices * sizeof(geom::Vertex), "mesh_vertices");

    for (uint32 idx = 0; idx < ai_mesh->mNumVertices; idx++) {
        geom::Vertex *vertex = &mesh->vertices[idx];
        *vertex = {};

        v4 raw_vertex_pos = v4(
            ai_mesh->mVertices[idx].x,
            ai_mesh->mVertices[idx].y,
            ai_mesh->mVertices[idx].z,
            1.0f);
        vertex->position = v3(mesh->transform * raw_vertex_pos);

        v3 raw_vertex_normal = v3(
            ai_mesh->mNormals[idx].x,
            ai_mesh->mNormals[idx].y,
            ai_mesh->mNormals[idx].z);
        vertex->normal = normalize(normal_matrix * raw_vertex_normal);

        if (ai_mesh->mTextureCoords[0]) {
            vertex->tex_coords = v2(ai_mesh->mTextureCoords[0][idx].x, 1 - ai_mesh->mTextureCoords[0][idx].y);
        }
    }

    // Indices
    uint32 n_indices = 0;
    for (uint32 idx_face = 0; idx_face < ai_mesh->mNumFaces; idx_face++) {
        aiFace face = ai_mesh->mFaces[idx_face];
        n_indices += face.mNumIndices;
    }

    mesh->n_indices = n_indices;
    mesh->indices = (uint32*)memory::push(&mesh->temp_memory_pool,
        mesh->n_indices * sizeof(uint32), "mesh_indices");
    uint32 idx_index = 0;

    for (uint32 idx_face = 0; idx_face < ai_mesh->mNumFaces; idx_face++) {
        aiFace face = ai_mesh->mFaces[idx_face];
        for (
            uint32 idx_face_index = 0;
            idx_face_index < face.mNumIndices;
            idx_face_index++
        ) {
            mesh->indices[idx_index++] = face.mIndices[idx_face_index];
        }
    }

    // Bones
    assert(ai_mesh->mNumBones < MAX_N_BONES);
    anim::Component *animation_component = &model_loader->animation_component;
    range_named (idx_bone, 0, ai_mesh->mNumBones) {
        aiBone *ai_bone = ai_mesh->mBones[idx_bone];
        uint32 idx_found_bone = 0;
        bool32 did_find_bone = false;

        range_named (idx_animcomp_bone, 0, animation_component->n_bones) {
            if (pstr_eq(
                animation_component->bones[idx_animcomp_bone].name, ai_bone->mName.C_Str()
            )) {
                did_find_bone = true;
                idx_found_bone = idx_animcomp_bone;
                break;
            }
        }

        assert(did_find_bone);

        // NOTE: We really only need to do this once, but I honestly can't be
        // bothered to add some mechanism to check if we already set it, it would
        // just make things more complicated. We set it multiple times, whatever.
        // It's the same value anyway.
        animation_component->bones[idx_found_bone].offset =
            util::aimatrix4x4_to_glm(&ai_bone->mOffsetMatrix);

        range_named (idx_weight, 0, ai_bone->mNumWeights) {
            uint32 vertex_idx = ai_bone->mWeights[idx_weight].mVertexId;
            real32 weight = ai_bone->mWeights[idx_weight].mWeight;
            assert(vertex_idx < mesh->n_vertices);
            range_named (idx_vertex_weight, 0, MAX_N_BONES_PER_VERTEX) {
                // Put it in the next free space, if there is any.
                if (mesh->vertices[vertex_idx].bone_weights[idx_vertex_weight] == 0) {
                    mesh->vertices[vertex_idx].bone_idxs[idx_vertex_weight] = idx_found_bone;
                    mesh->vertices[vertex_idx].bone_weights[idx_vertex_weight] = weight;
                    break;
                }
            }
        }
    }
}


void
models::load_node(
    ModelLoader *model_loader,
    aiNode *node, const aiScene *scene,
    m4 accumulated_transform, Pack indices_pack
) {
    m4 node_transform = util::aimatrix4x4_to_glm(&node->mTransformation);
    m4 transform = accumulated_transform * node_transform;

    range (0, node->mNumMeshes) {
        aiMesh *ai_mesh = scene->mMeshes[node->mMeshes[idx]];
        geom::Mesh *mesh = &model_loader->meshes[model_loader->n_meshes++];
        *mesh = {};
        load_mesh(mesh, ai_mesh, scene, model_loader, transform, indices_pack);
    }

    range (0, node->mNumChildren) {
        Pack new_indices_pack = indices_pack;
        // NOTE: We can only store 4 bits per pack element. Our indices can be way
        // bigger than that, but that's fine. We don't need that much precision.
        // Just smash the number down to a uint8.
        pack::push(&new_indices_pack, (uint8)idx);
        load_node(model_loader, node->mChildren[idx], scene, transform, new_indices_pack);
    }
}


void
models::load_model_from_file(ModelLoader *model_loader)
{
    // NOTE: This function stores its vertex data in the MemoryPool for each
    // mesh, and so is intended to be called from a separate thread.
    char full_path[MAX_PATH] = {};
    pstr_vcat(full_path, MAX_PATH, MODEL_DIR, model_loader->model_path, NULL);

    START_TIMER(assimp_import);
    auto flags =
        aiProcess_Triangulate
        | aiProcess_JoinIdenticalVertices
        | aiProcess_SortByPType
        | aiProcess_GenNormals
        | aiProcess_FlipUVs
        // NOTE: This might break something in the future, let's look out for it.
        | aiProcess_OptimizeMeshes
        // NOTE: Use with caution, goes full YOLO.
        /* aiProcess_OptimizeGraph */
        /* | aiProcess_CalcTangentSpace */
        ;
    const aiScene *scene = aiImportFile(full_path, flags);
    END_TIMER(assimp_import);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        logs::fatal("assimp error: %s", aiGetErrorString());
        return;
    }

    anim::Component *animation_component = &model_loader->animation_component;
    load_bones(animation_component, scene);
    load_node(model_loader, scene->mRootNode, scene, m4(1.0f), 0ULL);
    load_animations(animation_component, scene);
    aiReleaseImport(scene);

    model_loader->state = ModelLoaderState::mesh_data_loaded;
}


void
models::load_model_from_data(ModelLoader *model_loader)
{
    // NOTE: This function sets up mesh vertex buffers directly, and so is
    // intended to be called from the main OpenGL thread.
    MemoryPool temp_memory_pool = {};

    geom::Vertex *vertex_data = nullptr;
    uint32 n_vertices = 0;
    uint32 *index_data = nullptr;
    uint32 n_indices = 0;
    GLenum mode = 0;

    if (pstr_eq(model_loader->model_path, "builtin:axes")) {
        vertex_data = (geom::Vertex*)AXES_VERTICES;
        n_vertices = 6;
        index_data = nullptr;
        n_indices = 0;
        mode = GL_LINES;
    } else if (pstr_eq(model_loader->model_path, "builtin:ocean")) {
        geom::make_plane(&temp_memory_pool,
            200, 200, 800, 800, &n_vertices, &n_indices, &vertex_data, &index_data);
        mode = GL_TRIANGLES;
    } else if (pstr_eq(model_loader->model_path, "builtin:skysphere")) {
        geom::make_sphere(&temp_memory_pool,
            64, 64, &n_vertices, &n_indices, &vertex_data, &index_data);
        mode = GL_TRIANGLE_STRIP;
    } else if (
        pstr_starts_with(model_loader->model_path, "builtin:screenquad")
    ) {
        vertex_data = (geom::Vertex*)SCREENQUAD_VERTICES;
        n_vertices = 6;
        index_data = nullptr;
        n_indices = 0;
        mode = GL_TRIANGLES;
    } else {
        logs::fatal("Could not find builtin model: %s", model_loader->model_path);
    }

    geom::Mesh *mesh = &model_loader->meshes[model_loader->n_meshes++];
    *mesh = {};
    mesh->transform = m4(1.0f);
    mesh->mode = mode;
    mesh->n_vertices = n_vertices;
    mesh->n_indices = n_indices;
    mesh->indices_pack = 0UL;

    geom::setup_mesh_vertex_buffers(mesh, vertex_data, n_vertices, index_data, n_indices);
    model_loader->state = ModelLoaderState::vertex_buffers_set_up;

    memory::destroy_memory_pool(&temp_memory_pool);
}
