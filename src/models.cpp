/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

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


namespace models {
  pny_internal void make_plane(
    MemoryPool *memory_pool,
    uint32 x_size, uint32 z_size,
    uint32 n_x_segments, uint32 n_z_segments,
    uint32 *n_vertices, uint32 *n_indices,
    Vertex **vertex_data, uint32 **index_data
  ) {
    *n_vertices = 0;
    *n_indices = 0;

    uint32 n_total_vertices = (n_x_segments + 1) * (n_z_segments + 1);
    uint32 index_data_length = (n_x_segments) * (n_z_segments) * 6;

    *vertex_data = (Vertex*)memory::push(
      memory_pool, sizeof(Vertex) * n_total_vertices, "plane_vertex_data"
    );
    *index_data = (uint32*)memory::push(
      memory_pool, sizeof(uint32) * index_data_length, "plane_index_data"
    );

    for (uint32 idx_x = 0; idx_x <= n_x_segments; idx_x++) {
      for (uint32 idx_z = 0; idx_z <= n_z_segments; idx_z++) {
        real32 x_segment = (real32)idx_x / (real32)n_x_segments;
        real32 z_segment = (real32)idx_z / (real32)n_z_segments;
        real32 x_pos = x_segment * x_size - (x_size / 2);
        real32 y_pos = 0;
        real32 z_pos = z_segment * z_size - (z_size / 2);

        (*vertex_data)[(*n_vertices)++] = {
          .position = { x_pos, y_pos, z_pos },
          .normal = { 0.0f, 1.0f, 0.0f },
          .tex_coords = { x_segment, z_segment },
        };
      }
    }

    // NOTE: Counterclockwise winding order. I could swear this code is CW
    // order though. Not sure where the confusion happens.
    for (uint32 idx_x = 0; idx_x < n_x_segments; idx_x++) {
      for (uint32 idx_z = 0; idx_z < n_z_segments; idx_z++) {
        // This current vertex.
        (*index_data)[(*n_indices)++] = (idx_x * (n_z_segments + 1)) + idx_z;
        // Next row, right of this one.
        (*index_data)[(*n_indices)++] = ((idx_x + 1) * (n_z_segments + 1)) + idx_z + 1;
        // Next row, under this one.
        (*index_data)[(*n_indices)++] = ((idx_x + 1) * (n_z_segments + 1)) + idx_z;

        // This current vertex.
        (*index_data)[(*n_indices)++] = (idx_x * (n_z_segments + 1)) + idx_z;
        // This row, right of this one.
        (*index_data)[(*n_indices)++] = (idx_x * (n_z_segments + 1)) + idx_z + 1;
        // Next row, right of this one.
        (*index_data)[(*n_indices)++] = ((idx_x + 1) * (n_z_segments + 1)) + idx_z + 1;
      }
    }
  }


  pny_internal void make_sphere(
    MemoryPool *memory_pool,
    uint32 n_x_segments, uint32 n_y_segments,
    uint32 *n_vertices, uint32 *n_indices,
    Vertex **vertex_data, uint32 **index_data
  ) {
    *n_vertices = 0;
    *n_indices = 0;

    uint32 total_n_vertices = (n_x_segments + 1) * (n_y_segments + 1);
    uint32 index_data_length = (n_x_segments + 1) * (n_y_segments) * 2;

    *vertex_data = (Vertex*)memory::push(
      memory_pool, sizeof(Vertex) * total_n_vertices, "sphere_vertex_data"
    );
    *index_data = (uint32*)memory::push(
      memory_pool, sizeof(uint32) * index_data_length, "sphere_index_data"
    );

    for (uint32 y = 0; y <= n_y_segments; y++) {
      for (uint32 x = 0; x <= n_x_segments; x++) {
        real32 x_segment = (real32)x / (real32)n_x_segments;
        real32 y_segment = (real32)y / (real32)n_y_segments;
        real32 x_pos = cos(x_segment * 2.0f * PI32) * sin(y_segment * PI32);
        real32 y_pos = cos(y_segment * PI32);
        real32 z_pos = sin(x_segment * 2.0f * PI32) * sin(y_segment * PI32);

        (*vertex_data)[(*n_vertices)++] = {
          .position = {x_pos, y_pos, z_pos},
          .normal = {x_pos, y_pos, z_pos},
          .tex_coords = {x_segment, y_segment},
        };
      }
    }

    for (uint32 y = 0; y < n_y_segments; y++) {
      if (y % 2 == 0) {
        /* for (int32 x = n_x_segments; x >= 0; x--) { */
        for (uint32 x = 0; x <= n_x_segments; x++) {
          (*index_data)[(*n_indices)++] = (y + 1) * (n_x_segments + 1) + x;
          (*index_data)[(*n_indices)++] = y * (n_x_segments + 1) + x;
        }
      } else {
        /* for (uint32 x = 0; x <= n_x_segments; x++) { */
        for (int32 x = n_x_segments; x >= 0; x--) {
          (*index_data)[(*n_indices)++] = y * (n_x_segments + 1) + x;
          (*index_data)[(*n_indices)++] = (y + 1) * (n_x_segments + 1) + x;
        }
      }
    }
  }


  pny_internal void setup_mesh_vertex_buffers(
    Mesh *mesh,
    Vertex *vertex_data, uint32 n_vertices,
    uint32 *index_data, uint32 n_indices
  ) {
    assert(vertex_data && n_vertices > 0);

    uint32 vertex_size = sizeof(Vertex);
    uint32 index_size = sizeof(uint32);

    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    glBindVertexArray(mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(
      GL_ARRAY_BUFFER, vertex_size * n_vertices,
      vertex_data, GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(
      GL_ELEMENT_ARRAY_BUFFER, index_size * n_indices,
      index_data, GL_STATIC_DRAW
    );

    uint32 location;

    location = 0;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
      location, 3, GL_FLOAT, GL_FALSE, vertex_size,
      (void*)offsetof(Vertex, position)
    );

    location = 1;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
      location, 3, GL_FLOAT, GL_FALSE, vertex_size,
      (void*)offsetof(Vertex, normal)
    );

    location = 2;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
      location, 2, GL_FLOAT, GL_FALSE, vertex_size,
      (void*)offsetof(Vertex, tex_coords)
    );

    location = 3;
    glEnableVertexAttribArray(location);
    glVertexAttribIPointer(
      location, MAX_N_BONES_PER_VERTEX, GL_INT, vertex_size,
      (void*)offsetof(Vertex, bone_idxs)
    );

    location = 4;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(
      location, MAX_N_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, vertex_size,
      (void*)offsetof(Vertex, bone_weights)
    );
  }


  pny_internal bool32 is_bone_only_node(aiNode *node) {
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


  pny_internal aiNode* find_root_bone(const aiScene *scene) {
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


  pny_internal void add_bone_tree_to_animation_component(
    AnimationComponent *animation_component,
    aiNode *node,
    uint32 idx_parent
  ) {
    uint32 idx_new_bone = animation_component->n_bones;
    animation_component->bones[idx_new_bone] = {
      .idx_parent = idx_parent,
      // NOTE: offset is added later, since we don't have the aiBone at this stage.
    };
    pstr_copy(
      animation_component->bones[idx_new_bone].name,
      MAX_NODE_NAME_LENGTH,
      node->mName.C_Str()
    );
    animation_component->n_bones++;

    range (0, node->mNumChildren) {
      add_bone_tree_to_animation_component(
        animation_component,
        node->mChildren[idx],
        idx_new_bone
      );
    }
  }


  pny_internal void load_bones(
    AnimationComponent *animation_component,
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


  pny_internal void load_animations(
    AnimationComponent *animation_component,
    const aiScene *scene,
    BoneMatrixPool *bone_matrix_pool
  ) {
    m4 scene_root_transform =
      util::aimatrix4x4_to_glm(&scene->mRootNode->mTransformation);
    m4 inverse_scene_root_transform = inverse(scene_root_transform);

    animation_component->n_animations = scene->mNumAnimations;
    range_named (idx_animation, 0, scene->mNumAnimations) {
      Animation *animation = &animation_component->animations[idx_animation];
      aiAnimation *ai_animation = scene->mAnimations[idx_animation];

      *animation = {
        .duration = ai_animation->mDuration * ai_animation->mTicksPerSecond,
        .idx_bone_matrix_set = anim::push_to_bone_matrix_pool(bone_matrix_pool),
      };
      pstr_copy(
        animation->name,
        MAX_NODE_NAME_LENGTH,
        ai_animation->mName.C_Str()
      );

      // Calculate bone matrices.
      // NOTE: We do not finalise the bone matrices at this stage!
      // The matrices in local form are still needed for the children.
      range_named(idx_bone, 0, animation_component->n_bones) {
        Bone *bone = &animation_component->bones[idx_bone];

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

        anim::make_bone_matrices_for_animation_bone(
          animation_component,
          ai_animation->mChannels[found_channel_idx],
          idx_animation,
          idx_bone,
          bone_matrix_pool
        );
      }

      // Finalise bone matrices.
      // NOTE: Now that we've calculated all the bone matrices for this
      // animation, we can finalise them.
      range_named(idx_bone, 0, animation_component->n_bones) {
        Bone *bone = &animation_component->bones[idx_bone];

        range_named (idx_anim_key, 0, bone->n_anim_keys) {
          // #slow: We could avoid this multiplication here.
          m4 *bone_matrix = anim::get_bone_matrix(
            bone_matrix_pool,
            animation->idx_bone_matrix_set,
            idx_bone,
            idx_anim_key
          );

          *bone_matrix =
            scene_root_transform *
            *bone_matrix *
            bone->offset *
            inverse_scene_root_transform;
        }
      }
    }
  }


  pny_internal void load_mesh(
    Mesh *mesh,
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
    mesh->vertices = (Vertex*)memory::push(
      &mesh->temp_memory_pool,
      mesh->n_vertices * sizeof(Vertex),
      "mesh_vertices"
    );

    for (uint32 idx = 0; idx < ai_mesh->mNumVertices; idx++) {
      Vertex *vertex = &mesh->vertices[idx];
      *vertex = {};

      vertex->position = v3(
        mesh->transform *
        v4(
          ai_mesh->mVertices[idx].x,
          ai_mesh->mVertices[idx].y,
          ai_mesh->mVertices[idx].z,
          1.0f
        )
      );

      vertex->normal = normalize(
        normal_matrix *
        v3(
          ai_mesh->mNormals[idx].x,
          ai_mesh->mNormals[idx].y,
          ai_mesh->mNormals[idx].z
        )
      );

      if (ai_mesh->mTextureCoords[0]) {
        vertex->tex_coords = v2(
          ai_mesh->mTextureCoords[0][idx].x,
          1 - ai_mesh->mTextureCoords[0][idx].y
        );
      }
    }

    // Indices
    uint32 n_indices = 0;
    for (uint32 idx_face = 0; idx_face < ai_mesh->mNumFaces; idx_face++) {
      aiFace face = ai_mesh->mFaces[idx_face];
      n_indices += face.mNumIndices;
    }

    mesh->n_indices = n_indices;
    mesh->indices = (uint32*)memory::push(
      &mesh->temp_memory_pool,
      mesh->n_indices * sizeof(uint32),
      "mesh_indices"
    );
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
    AnimationComponent *animation_component = &model_loader->animation_component;
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


  pny_internal void destroy_mesh(Mesh *mesh) {
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->ebo);
  }


  pny_internal void load_node(
    ModelLoader *model_loader,
    aiNode *node, const aiScene *scene,
    m4 accumulated_transform, Pack indices_pack
  ) {
    m4 node_transform = util::aimatrix4x4_to_glm(&node->mTransformation);
    m4 transform = accumulated_transform * node_transform;

    range (0, node->mNumMeshes) {
      aiMesh *ai_mesh = scene->mMeshes[node->mMeshes[idx]];
      Mesh *mesh = &model_loader->meshes[model_loader->n_meshes++];
      *mesh = {};
      load_mesh(
        mesh,
        ai_mesh,
        scene,
        model_loader,
        transform,
        indices_pack
      );
    }

    range (0, node->mNumChildren) {
      Pack new_indices_pack = indices_pack;
      // NOTE: We can only store 4 bits per pack element. Our indices can be way
      // bigger than that, but that's fine. We don't need that much precision.
      // Just smash the number down to a uint8.
      pack::push(&new_indices_pack, (uint8)idx);
      load_node(
        model_loader, node->mChildren[idx], scene, transform, new_indices_pack
      );
    }
  }


  pny_internal void load_model_from_file(
    ModelLoader *model_loader,
    BoneMatrixPool *bone_matrix_pool
  ) {
    // NOTE: This function stores its vertex data in the MemoryPool for each
    // mesh, and so is intended to be called from a separate thread.
    char full_path[MAX_PATH] = {};
    pstr_vcat(full_path, MAX_PATH, MODEL_DIR, model_loader->model_path, NULL);

    START_TIMER(assimp_import);
    const aiScene *scene = aiImportFile(
      full_path,
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
    );
    END_TIMER(assimp_import);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
      logs::fatal("assimp error: %s", aiGetErrorString());
      return;
    }

    AnimationComponent *animation_component = &model_loader->animation_component;
    load_bones(animation_component, scene);
    load_node(model_loader, scene->mRootNode, scene, m4(1.0f), 0ULL);
    load_animations(animation_component, scene, bone_matrix_pool);
    aiReleaseImport(scene);

    model_loader->state = ModelLoaderState::mesh_data_loaded;
  }


  pny_internal void load_model_from_data(
    ModelLoader *model_loader
  ) {
    // NOTE: This function sets up mesh vertex buffers directly, and so is
    // intended to be called from the main OpenGL thread.
    MemoryPool temp_memory_pool = {};

    Vertex *vertex_data = nullptr;
    uint32 n_vertices = 0;
    uint32 *index_data = nullptr;
    uint32 n_indices = 0;
    GLenum mode = 0;

    if (pstr_eq(model_loader->model_path, "builtin:axes")) {
      vertex_data = (Vertex*)AXES_VERTICES;
      n_vertices = 6;
      index_data = nullptr;
      n_indices = 0;
      mode = GL_LINES;
    } else if (pstr_eq(model_loader->model_path, "builtin:ocean")) {
      make_plane(
        &temp_memory_pool,
        200, 200,
        800, 800,
        &n_vertices, &n_indices,
        &vertex_data, &index_data
      );
      mode = GL_TRIANGLES;
    } else if (pstr_eq(model_loader->model_path, "builtin:skysphere")) {
      make_sphere(
        &temp_memory_pool,
        64, 64,
        &n_vertices, &n_indices,
        &vertex_data, &index_data
      );
      mode = GL_TRIANGLE_STRIP;
    } else if (
      pstr_starts_with(model_loader->model_path, "builtin:screenquad")
    ) {
      vertex_data = (Vertex*)SCREENQUAD_VERTICES;
      n_vertices = 6;
      index_data = nullptr;
      n_indices = 0;
      mode = GL_TRIANGLES;
    } else {
      logs::fatal("Could not find builtin model: %s", model_loader->model_path);
    }

    Mesh *mesh = &model_loader->meshes[model_loader->n_meshes++];
    *mesh = {};
    mesh->transform = m4(1.0f);
    mesh->mode = mode;
    mesh->n_vertices = n_vertices;
    mesh->n_indices = n_indices;
    mesh->indices_pack = 0UL;

    setup_mesh_vertex_buffers(mesh, vertex_data, n_vertices, index_data, n_indices);
    model_loader->state = ModelLoaderState::vertex_buffers_set_up;

    memory::destroy_memory_pool(&temp_memory_pool);
  }


  bool32 is_mesh_valid(Mesh *mesh) {
    return mesh->vao > 0;
  }
}


const char* models::render_pass_to_string(RenderPass render_pass) {
  if (render_pass == RenderPass::none) {
    return "none";
  } else if (render_pass == RenderPass::shadowcaster) {
    return "shadowcaster";
  } else if (render_pass == RenderPass::deferred) {
    return "deferred";
  } else if (render_pass == RenderPass::forward_depth) {
    return "forward_depth";
  } else if (render_pass == RenderPass::forward_nodepth) {
    return "forward_nodepth";
  } else if (render_pass == RenderPass::forward_skybox) {
    return "forward_skybox";
  } else if (render_pass == RenderPass::lighting) {
    return "lighting";
  } else if (render_pass == RenderPass::postprocessing) {
    return "postprocessing";
  } else if (render_pass == RenderPass::preblur) {
    return "preblur";
  } else if (render_pass == RenderPass::blur1) {
    return "blur1";
  } else if (render_pass == RenderPass::blur2) {
    return "blur2";
  } else if (render_pass == RenderPass::renderdebug) {
    return "renderdebug";
  } else {
    logs::error("Don't know how to convert RenderPass to string: %d", render_pass);
    return "<unknown>";
  }
}


RenderPass models::render_pass_from_string(const char* str) {
  if (pstr_eq(str, "none")) {
    return RenderPass::none;
  } else if (pstr_eq(str, "shadowcaster")) {
    return RenderPass::shadowcaster;
  } else if (pstr_eq(str, "deferred")) {
    return RenderPass::deferred;
  } else if (pstr_eq(str, "forward_depth")) {
    return RenderPass::forward_depth;
  } else if (pstr_eq(str, "forward_nodepth")) {
    return RenderPass::forward_nodepth;
  } else if (pstr_eq(str, "forward_skybox")) {
    return RenderPass::forward_skybox;
  } else if (pstr_eq(str, "lighting")) {
    return RenderPass::lighting;
  } else if (pstr_eq(str, "postprocessing")) {
    return RenderPass::postprocessing;
  } else if (pstr_eq(str, "preblur")) {
    return RenderPass::preblur;
  } else if (pstr_eq(str, "blur1")) {
    return RenderPass::blur1;
  } else if (pstr_eq(str, "blur2")) {
    return RenderPass::blur2;
  } else if (pstr_eq(str, "renderdebug")) {
    return RenderPass::renderdebug;
  } else {
    logs::fatal("Could not parse RenderPass: %s", str);
    return RenderPass::none;
  }
}


bool32 models::prepare_model_loader_and_check_if_done(
  ModelLoader *model_loader,
  PersistentPbo *persistent_pbo,
  TextureNamePool *texture_name_pool,
  Queue<Task> *task_queue,
  BoneMatrixPool *bone_matrix_pool
) {
  if (model_loader->state == ModelLoaderState::initialized) {
    if (pstr_starts_with(model_loader->model_path, "builtin:")) {
      logs::error(
        "Found model with builtin model_path for which no vertex data was loaded."
      );
      return false;
    }
    task_queue->push({
      .fn = (TaskFn)load_model_from_file,
      .argument_1 = (void*)model_loader,
      .argument_2 = (void*)bone_matrix_pool,
    });
    model_loader->state = ModelLoaderState::mesh_data_being_loaded;
  }

  if (model_loader->state == ModelLoaderState::mesh_data_being_loaded) {
    // Wait. The task will progress this for us.
  }

  if (model_loader->state == ModelLoaderState::mesh_data_loaded) {
    for (uint32 idx = 0; idx < model_loader->n_meshes; idx++) {
      Mesh *mesh = &model_loader->meshes[idx];
      setup_mesh_vertex_buffers(
        mesh,
        mesh->vertices, mesh->n_vertices,
        mesh->indices, mesh->n_indices
      );
      memory::destroy_memory_pool(&mesh->temp_memory_pool);
    }
    model_loader->state = ModelLoaderState::vertex_buffers_set_up;
  }

  if (model_loader->state == ModelLoaderState::vertex_buffers_set_up) {
    // Set material names for each mesh
    range_named (idx_material, 0, model_loader->n_material_names) {
      range_named (idx_mesh, 0, model_loader->n_meshes) {
        Mesh *mesh = &model_loader->meshes[idx_mesh];
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
          pstr_copy(
            mesh->material_name,
            MAX_COMMON_NAME_LENGTH,
            model_loader->material_names[idx_material]
          );
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


bool32 models::prepare_entity_loader_and_check_if_done(
  EntityLoader *entity_loader,
  EntitySet *entity_set,
  ModelLoader *model_loader,
  DrawableComponentSet *drawable_component_set,
  SpatialComponentSet *spatial_component_set,
  LightComponentSet *light_component_set,
  BehaviorComponentSet *behavior_component_set,
  AnimationComponentSet *animation_component_set,
  physics::ComponentSet *physics_component_set
) {
  if (entity_loader->state == EntityLoaderState::initialized) {
    // Before we can create entities, we need this entity's models to have
    // been loaded.
    if (model_loader->state != ModelLoaderState::complete) {
      return false;
    }

    SpatialComponent *spatial_component =
      spatial_component_set->components[entity_loader->entity_handle];
    *spatial_component = entity_loader->spatial_component;
    spatial_component->entity_handle = entity_loader->entity_handle;

    LightComponent *light_component =
      light_component_set->components[entity_loader->entity_handle];
    *light_component = entity_loader->light_component;
    light_component->entity_handle = entity_loader->entity_handle;

    BehaviorComponent *behavior_component =
      behavior_component_set->components[entity_loader->entity_handle];
    *behavior_component = entity_loader->behavior_component;
    behavior_component->entity_handle = entity_loader->entity_handle;

    AnimationComponent *animation_component =
      animation_component_set->components[entity_loader->entity_handle];
    *animation_component = model_loader->animation_component;
    animation_component->entity_handle = entity_loader->entity_handle;

    physics::Component *physics_component =
      physics_component_set->components[entity_loader->entity_handle];
    *physics_component = entity_loader->physics_component;
    physics_component->entity_handle = entity_loader->entity_handle;

    // DrawableComponent
    if (model_loader->n_meshes == 1) {
      DrawableComponent *drawable_component =
        drawable_component_set->components[entity_loader->entity_handle];
      assert(drawable_component);
      *drawable_component = {
        .entity_handle = entity_loader->entity_handle,
        .mesh = model_loader->meshes[0],
        .target_render_pass = entity_loader->render_pass,
      };
    } else if (model_loader->n_meshes > 1) {
      for (uint32 idx = 0; idx < model_loader->n_meshes; idx++) {
        Mesh *mesh = &model_loader->meshes[idx];

        Entity *child_entity = entities::add_entity_to_set(
          entity_set,
          entity_loader->name
        );

        if (spatial::is_spatial_component_valid(&entity_loader->spatial_component)) {
          SpatialComponent *child_spatial_component =
            spatial_component_set->components[child_entity->handle];
          assert(child_spatial_component);
          *child_spatial_component = {
            .entity_handle = child_entity->handle,
            .position = v3(0.0f),
            .rotation = glm::angleAxis(radians(0.0f), v3(0.0f)),
            .scale = v3(0.0f),
            .parent_entity_handle = entity_loader->entity_handle,
          };
        }

        DrawableComponent *drawable_component =
          drawable_component_set->components[child_entity->handle];
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


bool32 models::is_model_loader_valid(ModelLoader *model_loader) {
  return model_loader->state != ModelLoaderState::empty;
}


bool32 models::is_entity_loader_valid(EntityLoader *entity_loader) {
  return entity_loader->state != EntityLoaderState::empty;
}


void models::add_material_to_model_loader(
  ModelLoader *model_loader,
  char const *material_name
) {
  pstr_copy(
    model_loader->material_names[model_loader->n_material_names++],
    MAX_COMMON_NAME_LENGTH,
    material_name
  );
}


ModelLoader* models::init_model_loader(
  ModelLoader *model_loader,
  const char *model_path
) {
  assert(model_loader);
  pstr_copy(model_loader->model_path, MAX_PATH, model_path);

  model_loader->state = ModelLoaderState::initialized;

  if (pstr_starts_with(model_path, "builtin:")) {
    load_model_from_data(model_loader);
  }

  return model_loader;
}


EntityLoader* models::init_entity_loader(
  EntityLoader *entity_loader,
  const char *name,
  const char *model_path,
  RenderPass render_pass,
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


bool32 models::is_drawable_component_valid(DrawableComponent *drawable_component) {
  return is_mesh_valid(&drawable_component->mesh);
}


void models::destroy_drawable_component(DrawableComponent *drawable_component) {
  if (!is_drawable_component_valid(drawable_component)) {
    return;
  }
  destroy_mesh(&drawable_component->mesh);
}
