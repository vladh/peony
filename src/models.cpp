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
  for_range_named (idx_bone, 0, ai_mesh->mNumBones) {
    aiBone *ai_bone = ai_mesh->mBones[idx_bone];
    AnimationComponent *animation_component = &entity_loader->animation_component;
    uint32 idx_found_bone = 0;
    bool32 did_find_bone = false;

    for_range_named (idx_animcomp_bone, 0, animation_component->n_bones) {
      if (Str::eq(
        animation_component->bones[idx_animcomp_bone].name, ai_bone->mName.C_Str())
      ) {
        did_find_bone = true;
        idx_found_bone = idx_animcomp_bone;
        break;
      }
    }

    if (!did_find_bone) {
      idx_found_bone = animation_component->n_bones;
      animation_component->bones[idx_found_bone] = {
        .offset = Util::aimatrix4x4_to_glm(&ai_bone->mOffsetMatrix),
      };
      // TODO: Fix string handling, Bone.name is only 32 bytes big.
      strcpy(animation_component->bones[idx_found_bone].name, ai_bone->mName.C_Str());
      animation_component->n_bones++;
    }

    for_range_named (idx_weight, 0, ai_bone->mNumWeights) {
      uint32 vertex_idx = ai_bone->mWeights[idx_weight].mVertexId;
      real32 weight = ai_bone->mWeights[idx_weight].mWeight;
      assert(vertex_idx < mesh->n_vertices);
      for_range_named (idx_vertex_weight, 0, MAX_N_BONES_PER_VERTEX) {
        // Put it in the next free space, if there is any.
        if (mesh->vertices[vertex_idx].bone_idxs[idx_vertex_weight] == -1) {
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


void Models::load_animations(
  EntityLoader *entity_loader,
  const aiScene *scene
) {
  entity_loader->animation_component.n_animations = scene->mNumAnimations;
  for_range_named (idx_animation, 0, scene->mNumAnimations) {
    Animation *animation =
      &entity_loader->animation_component.animations[idx_animation];
    aiAnimation *ai_animation = scene->mAnimations[idx_animation];
    strcpy(animation->name, ai_animation->mName.C_Str());
    animation->duration = ai_animation->mDuration;
    animation->ticks_per_second = ai_animation->mTicksPerSecond;

    log_info(
      "animation %d for %s (dur %f)",
      idx_animation,
      entity_loader->name,
      animation->duration
    );

    animation->n_anim_channels = ai_animation->mNumChannels;
    for_range_named (idx_channel, 0, ai_animation->mNumChannels) {
      aiNodeAnim *ai_channel = ai_animation->mChannels[idx_channel];
      AnimChannel *channel = &animation->anim_channels[idx_channel];
      *channel = {
        .n_position_keys = ai_channel->mNumPositionKeys,
        .n_rotation_keys = ai_channel->mNumRotationKeys,
        .n_scaling_keys = ai_channel->mNumScalingKeys,
      };
      log_info(
        "channel %d: %d position, %d rotation, %d scaling",
        idx_channel,
        channel->n_position_keys,
        channel->n_rotation_keys,
        channel->n_scaling_keys
      );

      for_range_named (idx_key, 0, ai_channel->mNumPositionKeys) {
        channel->position_keys[idx_key] = {
          .position = Util::aiVector3D_to_glm(
            &ai_channel->mPositionKeys[idx_key].mValue
          ),
          .time = ai_channel->mPositionKeys[idx_key].mTime,
        };
      }

      for_range_named (idx_key, 0, ai_channel->mNumRotationKeys) {
        channel->rotation_keys[idx_key] = {
          .rotation = Util::aiQuaternion_to_glm(
            &ai_channel->mRotationKeys[idx_key].mValue
          ),
          .time = ai_channel->mRotationKeys[idx_key].mTime,
        };
      }

      for_range_named (idx_key, 0, ai_channel->mNumScalingKeys) {
        channel->scaling_keys[idx_key] = {
          .scale = Util::aiVector3D_to_glm(
            &ai_channel->mScalingKeys[idx_key].mValue
          ),
          .time = ai_channel->mScalingKeys[idx_key].mTime,
        };
      }
    }
  }
}


void Models::load_model(EntityLoader *entity_loader) {
  // If we're not loading from a file, all the data has already
  // been loaded previously, so we just need to handle the
  // shaders and textures and so on, so skip this.
  char full_path[MAX_PATH];
  strcpy(full_path, MODEL_DIR);
  strcat(full_path, entity_loader->path);

  if (entity_loader->model_source == ModelSource::file) {
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

    load_node(
      entity_loader, scene->mRootNode, scene, glm::mat4(1.0f), 0ULL
    );

    load_animations(
      entity_loader, scene
    );

    aiReleaseImport(scene);
  }

  entity_loader->state = EntityLoaderState::mesh_data_loaded;
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
  SpatialComponent *spatial_component = spatial_component_set->components.get(
    entity_loader->entity_handle
  );
  memcpy(
    spatial_component,
    &entity_loader->spatial_component,
    sizeof(SpatialComponent)
  );
  spatial_component->entity_handle = entity_loader->entity_handle;

  LightComponent *light_component = light_component_set->components.get(
    entity_loader->entity_handle
  );
  memcpy(
    light_component,
    &entity_loader->light_component,
    sizeof(LightComponent)
  );
  light_component->entity_handle = entity_loader->entity_handle;

  BehaviorComponent *behavior_component = behavior_component_set->components.get(
    entity_loader->entity_handle
  );
  memcpy(
    behavior_component,
    &entity_loader->behavior_component,
    sizeof(BehaviorComponent)
  );
  behavior_component->entity_handle = entity_loader->entity_handle;

  AnimationComponent *animation_component = animation_component_set->components.get(
    entity_loader->entity_handle
  );
  memcpy(
    animation_component,
    &entity_loader->animation_component,
    sizeof(AnimationComponent)
  );
  animation_component->entity_handle = entity_loader->entity_handle;

  // DrawableComponent
  if (entity_loader->n_meshes == 1) {
    DrawableComponent *drawable_component = drawable_component_set->components.get(
      entity_loader->entity_handle
    );
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
          spatial_component_set->components.get(child_entity->handle);
        assert(child_spatial_component);
        *child_spatial_component = {
          .entity_handle = child_entity->handle,
          .position = glm::vec3(0.0f),
          .rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f)),
          .scale = glm::vec3(0.0f),
          .parent_entity_handle = entity_loader->entity_handle,
        };
      }

      DrawableComponent *drawable_component = drawable_component_set->components.get(
        child_entity->handle
      );
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
  AnimationComponentSet *animation_component_set
) {
  if (entity_loader->state == EntityLoaderState::initialized) {
    task_queue->push({
      .type = TaskType::load_model,
      .target = {
        .entity_loader = entity_loader,
      },
      .persistent_pbo = nullptr,
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
    for (
      uint32 idx_material = 0; idx_material < entity_loader->n_materials; idx_material++
    ) {
      for (uint32 idx_mesh = 0; idx_mesh < entity_loader->n_meshes; idx_mesh++) {
        Mesh *mesh = &entity_loader->meshes[idx_mesh];
        uint8 mesh_number = pack_get(&mesh->indices_pack, 0);
        // For our model's mesh number `mesh_number`, we want to choose
        // material `idx_mesh` such that `mesh_number == idx_mesh`, i.e.
        // we choose the 4th material for mesh number 4.
        // However, if we have more meshes than materials, the extra
        // meshes all get material number 0.
        if (
          mesh_number == idx_material ||
          (mesh_number >= entity_loader->n_materials && idx_material == 0)
        ) {
          strcpy(mesh->material_name, entity_loader->material_names[idx_material]);
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
  const char *path,
  RenderPassFlag render_pass,
  EntityHandle entity_handle
) {
  assert(entity_loader);
  strcpy(entity_loader->name, name);
  entity_loader->state = EntityLoaderState::initialized;
  entity_loader->model_source = model_source;
  entity_loader->render_pass = render_pass;
  entity_loader->entity_handle = entity_handle;

  strcpy(entity_loader->path, path);
  // NOTE: We do not load ModelSource::file models here.
  // They will be loaded gradually in `::prepare_model_and_check_if_done()`.

  return entity_loader;
}


EntityLoader* Models::init_entity_loader(
  EntityLoader *entity_loader,
  ModelSource model_source,
  Vertex *vertex_data, uint32 n_vertices,
  uint32 *index_data, uint32 n_indices,
  const char *name,
  GLenum mode,
  RenderPassFlag render_pass,
  EntityHandle entity_handle
) {
  assert(entity_loader);
  strcpy(entity_loader->name, name);
  entity_loader->model_source = model_source;
  entity_loader->render_pass = render_pass;
  entity_loader->entity_handle = entity_handle;

  Mesh *mesh = &entity_loader->meshes[entity_loader->n_meshes++];
  *mesh = {};
  mesh->transform = glm::mat4(1.0f);
  mesh->mode = mode;
  mesh->n_vertices = n_vertices;
  mesh->n_indices = n_indices;
  mesh->indices_pack = 0UL;

  setup_mesh_vertex_buffers(mesh, vertex_data, n_vertices, index_data, n_indices);
  entity_loader->state = EntityLoaderState::vertex_buffers_set_up;

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
