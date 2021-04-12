void Models::setup_mesh_vertex_buffers(
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


bool32 Models::is_bone_only_node(aiNode *node) {
  if (node->mNumMeshes > 0) {
    return false;
  }
  bool32 have_we_found_it = true;
  for_range (0, node->mNumChildren) {
    if (!is_bone_only_node(node->mChildren[idx])) {
      have_we_found_it = false;
    }
  }
  return have_we_found_it;
}


aiNode* Models::find_root_bone(const aiScene *scene) {
  // NOTE: To find the root bone, we find the first-level node (direct child
  // of root node) whose entire descendent tree has no meshes, including the
  // leaf nodes. Is this a perfect way of finding the root bone? Probably
  // not. Is it good enough? Sure looks like it! :)
  aiNode *root_node = scene->mRootNode;

  for_range (0, root_node->mNumChildren) {
    aiNode *first_level_node = root_node->mChildren[idx];
    if (is_bone_only_node(first_level_node)) {
      return first_level_node;
    }
  }

  return nullptr;
}


void Models::add_bone_tree_to_animation_component(
  AnimationComponent *animation_component,
  aiNode *node,
  uint32 idx_parent
) {
  uint32 idx_new_bone = animation_component->n_bones;
  animation_component->bones[idx_new_bone] = {
    .idx_parent = idx_parent,
    // NOTE: offset is added later, since we don't have the aiBone at this stage.
  };
  strcpy(animation_component->bones[idx_new_bone].name, node->mName.C_Str());
  animation_component->n_bones++;

  for_range (0, node->mNumChildren) {
    add_bone_tree_to_animation_component(
      animation_component,
      node->mChildren[idx],
      idx_new_bone
    );
  }
}


void Models::load_bones(
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


void Models::load_animations(
  AnimationComponent *animation_component,
  const aiScene *scene,
  BoneMatrixPool *bone_matrix_pool
) {
  glm::mat4 scene_root_transform =
    Util::aimatrix4x4_to_glm(&scene->mRootNode->mTransformation);
  glm::mat4 inverse_scene_root_transform = glm::inverse(scene_root_transform);

  animation_component->n_animations = scene->mNumAnimations;
  for_range_named (idx_animation, 0, scene->mNumAnimations) {
    Animation *animation = &animation_component->animations[idx_animation];
    aiAnimation *ai_animation = scene->mAnimations[idx_animation];

    *animation = {
      .duration = ai_animation->mDuration * ai_animation->mTicksPerSecond,
      .idx_bone_matrix_set = EntitySets::push_to_bone_matrix_pool(bone_matrix_pool),
    };
    strcpy(animation->name, ai_animation->mName.C_Str());

    // Calculate bone matrices.
    // NOTE: We do not finalise the bone matrices at this stage!
    // The matrices in local form are still needed for the children.
    for_range_named(idx_bone, 0, animation_component->n_bones) {
      Bone *bone = &animation_component->bones[idx_bone];

      uint32 found_channel_idx = 0;
      bool32 did_find_channel = false;

      for_range_named (idx_channel, 0, ai_animation->mNumChannels) {
        aiNodeAnim *ai_channel = ai_animation->mChannels[idx_channel];
        if (Str::eq(ai_channel->mNodeName.C_Str(), bone->name)) {
          found_channel_idx = idx_channel;
          did_find_channel = true;
          break;
        }
      }

      if (!did_find_channel) {
        // No channel for this bone. Maybe it's just not animated. Skip it.
        continue;
      }

      EntitySets::make_bone_matrices_for_animation_bone(
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
    for_range_named(idx_bone, 0, animation_component->n_bones) {
      Bone *bone = &animation_component->bones[idx_bone];

      for_range_named (idx_anim_key, 0, bone->n_anim_keys) {
        // #slow: We could avoid this multiplication here.
        glm::mat4 *bone_matrix = EntitySets::get_bone_matrix(
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


void Models::load_mesh(
  Mesh *mesh,
  aiMesh *ai_mesh,
  const aiScene *scene,
  EntityLoader *entity_loader,
  glm::mat4 transform,
  Pack indices_pack
) {
  mesh->transform = transform;
  glm::mat3 normal_matrix = glm::mat3(glm::transpose(glm::inverse(transform)));
  mesh->mode = GL_TRIANGLES;

  mesh->indices_pack = indices_pack;

  // Vertices
  if (!ai_mesh->mNormals) {
    log_warning("Model does not have normals.");
  }

  mesh->n_vertices = ai_mesh->mNumVertices;
  mesh->vertices = (Vertex*)Memory::push(
    &mesh->temp_memory_pool,
    mesh->n_vertices * sizeof(Vertex),
    "mesh_vertices"
  );

  for (uint32 idx = 0; idx < ai_mesh->mNumVertices; idx++) {
    Vertex *vertex = &mesh->vertices[idx];
    *vertex = {};

    vertex->position = glm::vec3(
      mesh->transform *
      glm::vec4(
        ai_mesh->mVertices[idx].x,
        ai_mesh->mVertices[idx].y,
        ai_mesh->mVertices[idx].z,
        1.0f
      )
    );

    vertex->normal = glm::normalize(
      normal_matrix *
      glm::vec3(
        ai_mesh->mNormals[idx].x,
        ai_mesh->mNormals[idx].y,
        ai_mesh->mNormals[idx].z
      )
    );

    if (ai_mesh->mTextureCoords[0]) {
      vertex->tex_coords = glm::vec2(
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
  mesh->indices = (uint32*)Memory::push(
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
  AnimationComponent *animation_component = &entity_loader->animation_component;
  for_range_named (idx_bone, 0, ai_mesh->mNumBones) {
    aiBone *ai_bone = ai_mesh->mBones[idx_bone];
    uint32 idx_found_bone = 0;
    bool32 did_find_bone = false;

    for_range_named (idx_animcomp_bone, 0, animation_component->n_bones) {
      if (Str::eq(
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
      Util::aimatrix4x4_to_glm(&ai_bone->mOffsetMatrix);

    for_range_named (idx_weight, 0, ai_bone->mNumWeights) {
      uint32 vertex_idx = ai_bone->mWeights[idx_weight].mVertexId;
      real32 weight = ai_bone->mWeights[idx_weight].mWeight;
      assert(vertex_idx < mesh->n_vertices);
      for_range_named (idx_vertex_weight, 0, MAX_N_BONES_PER_VERTEX) {
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


void Models::destroy_mesh(Mesh *mesh) {
  glDeleteVertexArrays(1, &mesh->vao);
  glDeleteBuffers(1, &mesh->vbo);
  glDeleteBuffers(1, &mesh->ebo);
}


void Models::load_node(
  EntityLoader *entity_loader,
  aiNode *node, const aiScene *scene,
  glm::mat4 accumulated_transform, Pack indices_pack
) {
  glm::mat4 node_transform = Util::aimatrix4x4_to_glm(&node->mTransformation);
  glm::mat4 transform = accumulated_transform * node_transform;

  for_range (0, node->mNumMeshes) {
    aiMesh *ai_mesh = scene->mMeshes[node->mMeshes[idx]];
    Mesh *mesh = &entity_loader->meshes[entity_loader->n_meshes++];
    *mesh = {};
    load_mesh(
      mesh,
      ai_mesh,
      scene,
      entity_loader,
      transform,
      indices_pack
    );
  }

  for_range (0, node->mNumChildren) {
    Pack new_indices_pack = indices_pack;
    // NOTE: We can only store 4 bits per pack element. Our indices can be way
    // bigger than that, but that's fine. We don't need that much precision.
    // Just smash the number down to a uint8.
    pack_push(&new_indices_pack, (uint8)idx);
    load_node(
      entity_loader, node->mChildren[idx], scene, transform, new_indices_pack
    );
  }
}


void Models::load_model_from_file(
  EntityLoader *entity_loader,
  BoneMatrixPool *bone_matrix_pool
) {
  // NOTE: This function stores its vertex data in the MemoryPool for each
  // mesh, and so is intended to be called from a separate thread.
  char full_path[MAX_PATH];
  strcpy(full_path, MODEL_DIR);
  strcat(full_path, entity_loader->model_path_or_builtin_model_name);

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
    log_fatal("assimp error: %s", aiGetErrorString());
    return;
  }

  AnimationComponent *animation_component = &entity_loader->animation_component;

  load_bones(
    animation_component, scene
  );

  load_node(
    entity_loader, scene->mRootNode, scene, glm::mat4(1.0f), 0ULL
  );

  load_animations(
    animation_component, scene, bone_matrix_pool
  );

  aiReleaseImport(scene);

  entity_loader->state = EntityLoaderState::mesh_data_loaded;
}


void Models::load_model_from_data(
  EntityLoader *entity_loader
) {
  // NOTE: This function sets up mesh vertex buffers directly, and so is
  // intended to be called from the main OpenGL thread.
  MemoryPool temp_memory_pool = {};

  Vertex *vertex_data = nullptr;
  uint32 n_vertices = 0;
  uint32 *index_data = nullptr;
  uint32 n_indices = 0;
  GLenum mode = 0;

  if (Str::eq(entity_loader->model_path_or_builtin_model_name, "axes")) {
    vertex_data = (Vertex*)AXES_VERTICES;
    n_vertices = 6;
    index_data = nullptr;
    n_indices = 0;
    mode = GL_LINES;
  } else if (Str::eq(entity_loader->model_path_or_builtin_model_name, "ocean")) {
    Util::make_plane(
      &temp_memory_pool,
      200, 200,
      800, 800,
      &n_vertices, &n_indices,
      &vertex_data, &index_data
    );
    mode = GL_TRIANGLES;
  } else if (Str::eq(entity_loader->model_path_or_builtin_model_name, "skysphere")) {
    Util::make_sphere(
      &temp_memory_pool,
      64, 64,
      &n_vertices, &n_indices,
      &vertex_data, &index_data
    );
    mode = GL_TRIANGLE_STRIP;
  } else if (
    Str::starts_with(entity_loader->model_path_or_builtin_model_name, "screenquad")
  ) {
    vertex_data = (Vertex*)SCREENQUAD_VERTICES;
    n_vertices = 6;
    index_data = nullptr;
    n_indices = 0;
    mode = GL_TRIANGLES;
  } else {
    log_fatal(
      "Could not find builtin model: %s",
      entity_loader->model_path_or_builtin_model_name
    );
  }

  Mesh *mesh = &entity_loader->meshes[entity_loader->n_meshes++];
  *mesh = {};
  mesh->transform = glm::mat4(1.0f);
  mesh->mode = mode;
  mesh->n_vertices = n_vertices;
  mesh->n_indices = n_indices;
  mesh->indices_pack = 0UL;

  setup_mesh_vertex_buffers(mesh, vertex_data, n_vertices, index_data, n_indices);
  entity_loader->state = EntityLoaderState::vertex_buffers_set_up;

  Memory::destroy_memory_pool(&temp_memory_pool);
}


void Models::create_entities(
  EntityLoader *entity_loader,
  EntitySet *entity_set,
  DrawableComponentSet *drawable_component_set,
  SpatialComponentSet *spatial_component_set,
  LightComponentSet *light_component_set,
  BehaviorComponentSet *behavior_component_set,
  AnimationComponentSet *animation_component_set
) {
  SpatialComponent *spatial_component =
    spatial_component_set->components[ entity_loader->entity_handle];
  memcpy(
    spatial_component,
    &entity_loader->spatial_component,
    sizeof(SpatialComponent)
  );
  spatial_component->entity_handle = entity_loader->entity_handle;

  LightComponent *light_component =
    light_component_set->components[entity_loader->entity_handle];
  memcpy(
    light_component,
    &entity_loader->light_component,
    sizeof(LightComponent)
  );
  light_component->entity_handle = entity_loader->entity_handle;

  BehaviorComponent *behavior_component =
    behavior_component_set->components[entity_loader->entity_handle];
  memcpy(
    behavior_component,
    &entity_loader->behavior_component,
    sizeof(BehaviorComponent)
  );
  behavior_component->entity_handle = entity_loader->entity_handle;

  AnimationComponent *animation_component =
    animation_component_set->components[entity_loader->entity_handle];
  memcpy(
    animation_component,
    &entity_loader->animation_component,
    sizeof(AnimationComponent)
  );
  animation_component->entity_handle = entity_loader->entity_handle;

  // DrawableComponent
  if (entity_loader->n_meshes == 1) {
    DrawableComponent *drawable_component =
      drawable_component_set->components[entity_loader->entity_handle];
    assert(drawable_component);
    *drawable_component = {
      .entity_handle = entity_loader->entity_handle,
      .mesh = entity_loader->meshes[0],
      .target_render_pass = entity_loader->render_pass,
    };
  } else if (entity_loader->n_meshes > 1) {
    for (uint32 idx = 0; idx < entity_loader->n_meshes; idx++) {
      Mesh *mesh = &entity_loader->meshes[idx];

      Entity *child_entity = EntitySets::add_entity_to_set(
        entity_set,
        entity_loader->name
      );

      if (Entities::is_spatial_component_valid(&entity_loader->spatial_component)) {
        SpatialComponent *child_spatial_component =
          spatial_component_set->components[child_entity->handle];
        assert(child_spatial_component);
        *child_spatial_component = {
          .entity_handle = child_entity->handle,
          .position = glm::vec3(0.0f),
          .rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f)),
          .scale = glm::vec3(0.0f),
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
}


bool32 Models::prepare_model_and_check_if_done(
  EntityLoader *entity_loader,
  PersistentPbo *persistent_pbo,
  TextureNamePool *texture_name_pool,
  Queue<Task> *task_queue,
  EntitySet *entity_set,
  DrawableComponentSet *drawable_component_set,
  SpatialComponentSet *spatial_component_set,
  LightComponentSet *light_component_set,
  BehaviorComponentSet *behavior_component_set,
  AnimationComponentSet *animation_component_set,
  BoneMatrixPool *bone_matrix_pool
) {
  if (entity_loader->state == EntityLoaderState::initialized) {
    if (entity_loader->model_source != ModelSource::file) {
      log_error(
        "Found model with model_source=file for which no vertex data was loaded."
      );
      return false;
    }
    task_queue->push({
      .type = TaskType::load_model_from_data,
      .target = {
        .entity_loader = entity_loader,
      },
      .persistent_pbo = nullptr,
      .bone_matrix_pool = bone_matrix_pool,
    });
    entity_loader->state = EntityLoaderState::mesh_data_being_loaded;
  }

  if (entity_loader->state == EntityLoaderState::mesh_data_being_loaded) {
    // Wait. The task will progress this for us.
  }

  if (entity_loader->state == EntityLoaderState::mesh_data_loaded) {
    // Setup vertex buffers
    if (entity_loader->model_source == ModelSource::file) {
      for (uint32 idx = 0; idx < entity_loader->n_meshes; idx++) {
        Mesh *mesh = &entity_loader->meshes[idx];
        setup_mesh_vertex_buffers(
          mesh,
          mesh->vertices, mesh->n_vertices,
          mesh->indices, mesh->n_indices
        );
        Memory::destroy_memory_pool(&mesh->temp_memory_pool);
      }
    }
    entity_loader->state = EntityLoaderState::vertex_buffers_set_up;
  }

  if (entity_loader->state == EntityLoaderState::vertex_buffers_set_up) {
    // Set material names for each mesh
    for_range_named (idx_material, 0, entity_loader->material_names.length) {
      for_range_named (idx_mesh, 0, entity_loader->n_meshes) {
        Mesh *mesh = &entity_loader->meshes[idx_mesh];
        uint8 mesh_number = pack_get(&mesh->indices_pack, 0);
        // For our model's mesh number `mesh_number`, we want to choose
        // material `idx_mesh` such that `mesh_number == idx_mesh`, i.e.
        // we choose the 4th material for mesh number 4.
        // However, if we have more meshes than materials, the extra
        // meshes all get material number 0.
        if (
          mesh_number == idx_material ||
          (mesh_number >= entity_loader->material_names.length && idx_material == 0)
        ) {
          strcpy(mesh->material_name, *(entity_loader->material_names[idx_material]));
        }
      }
    }

    // Create any entities we might need to create
    create_entities(
      entity_loader,
      entity_set,
      drawable_component_set,
      spatial_component_set,
      light_component_set,
      behavior_component_set,
      animation_component_set
    );

    entity_loader->state = EntityLoaderState::complete;
  }

  if (entity_loader->state == EntityLoaderState::complete) {
    return true;
  }

  return false;
}


EntityLoader* Models::init_entity_loader(
  EntityLoader *entity_loader,
  ModelSource model_source,
  const char *name,
  const char *model_path_or_builtin_model_name,
  RenderPassFlag render_pass,
  EntityHandle entity_handle
) {
  assert(entity_loader);
  strcpy(entity_loader->name, name);
  entity_loader->state = EntityLoaderState::initialized;
  entity_loader->model_source = model_source;
  entity_loader->render_pass = render_pass;
  entity_loader->entity_handle = entity_handle;

  strcpy(
    entity_loader->model_path_or_builtin_model_name,
    model_path_or_builtin_model_name
  );

  if (model_source == ModelSource::data) {
    load_model_from_data(entity_loader);
  }

  return entity_loader;
}


bool32 Models::is_mesh_valid(Mesh *mesh) {
  return mesh->vao > 0;
}


bool32 Models::is_drawable_component_valid(
  DrawableComponent *drawable_component
) {
  return is_mesh_valid(&drawable_component->mesh);
}


void Models::destroy_drawable_component(DrawableComponent *drawable_component) {
  if (!is_drawable_component_valid(drawable_component)) {
    return;
  }
  destroy_mesh(&drawable_component->mesh);
}
