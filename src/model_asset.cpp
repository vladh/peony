void ModelAsset::setup_mesh_vertex_buffers_for_data_source(
  Mesh *mesh,
  real32 *vertex_data, uint32 n_vertices,
  uint32 *index_data, uint32 n_indices
) {
  /*
  NOTE: The structure of the vertices is as follows.
  [
    (3): pos_x, pos_y, pos_z,
    (3): normal_x, normal_y, normal_z,
    (2): tex_coords_x, tex_coords_y
    = (8)
  ]
  */
  uint32 vertex_size = sizeof(real32) * 8;
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
    location, 3, GL_FLOAT, GL_FALSE, vertex_size, (void*)(0)
  );

  location = 1;
  glEnableVertexAttribArray(location);
  glVertexAttribPointer(
    location, 3, GL_FLOAT, GL_FALSE, vertex_size, (void*)(3 * sizeof(real32))
  );

  location = 2;
  glEnableVertexAttribArray(location);
  glVertexAttribPointer(
    location, 2, GL_FLOAT, GL_FALSE, vertex_size, (void*)(6 * sizeof(real32))
  );
}


void ModelAsset::setup_mesh_vertex_buffers_for_file_source(
  Mesh *mesh, Array<Vertex> *vertices, Array<uint32> *indices
) {
  glGenVertexArrays(1, &mesh->vao);
  glGenBuffers(1, &mesh->vbo);
  glGenBuffers(1, &mesh->ebo);

  glBindVertexArray(mesh->vao);

  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
  glBufferData(
    GL_ARRAY_BUFFER, sizeof(Vertex) * vertices->size,
    vertices->get_items_ptr(), GL_STATIC_DRAW
  );

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * indices->size,
    indices->get_items_ptr(), GL_STATIC_DRAW
  );

  uint32 location;

  location = 0;
  glEnableVertexAttribArray(location);
  glVertexAttribPointer(
    location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0
  );

  location = 1;
  glEnableVertexAttribArray(location);
  glVertexAttribPointer(
    location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)
  );

  location = 2;
  glEnableVertexAttribArray(location);
  glVertexAttribPointer(
    location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords)
  );
}


void ModelAsset::load_mesh(
  Memory *memory, Mesh *mesh, aiMesh *mesh_data, const aiScene *scene,
  glm::mat4 transform, Pack indices_pack
) {
  mesh->transform = transform;
  glm::mat3 normal_matrix = glm::mat3(glm::transpose(glm::inverse(transform)));
  mesh->texture_set = nullptr;
  mesh->mode = GL_TRIANGLES;

  mesh->indices_pack = indices_pack;

  // Vertices
  // NOTE: We're duplicating the vertex and index data because the format
  // assimp gives us the data in is not something we can directly use.
  // This is probably quite wasteful, but I haven't figured out a way to
  // elegantly use the data from assimp directly, and I don't think it's
  // possible.
  // TODO: Change to a memory pool we can somehow clear later.
  mesh->vertices = Array<Vertex>(
    &memory->asset_memory_pool, mesh_data->mNumVertices, "mesh_vertices"
  );

  if (!mesh_data->mNormals) {
    log_warning("Model does not have normals.");
  }

  mesh->n_vertices = mesh_data->mNumVertices;

  for (uint32 idx = 0; idx < mesh_data->mNumVertices; idx++) {
    Vertex *vertex = mesh->vertices.push();

    glm::vec3 position;
    position.x = mesh_data->mVertices[idx].x;
    position.y = mesh_data->mVertices[idx].y;
    position.z = mesh_data->mVertices[idx].z;
    vertex->position = glm::vec3(mesh->transform * glm::vec4(position, 1.0));

    glm::vec3 normal;
    normal.x = mesh_data->mNormals[idx].x;
    normal.y = mesh_data->mNormals[idx].y;
    normal.z = mesh_data->mNormals[idx].z;
    vertex->normal = glm::normalize(normal_matrix * normal);

    if (mesh_data->mTextureCoords[0]) {
      glm::vec2 tex_coords;
      tex_coords.x = mesh_data->mTextureCoords[0][idx].x;
      tex_coords.y = 1 - mesh_data->mTextureCoords[0][idx].y;
      vertex->tex_coords = tex_coords;
    } else {
      vertex->tex_coords = glm::vec2(0.0f, 0.0f);
    }
  }

  // Indices
  uint32 n_indices = 0;
  for (uint32 idx_face = 0; idx_face < mesh_data->mNumFaces; idx_face++) {
    aiFace face = mesh_data->mFaces[idx_face];
    n_indices += face.mNumIndices;
  }

  mesh->n_indices = n_indices;
  mesh->indices = Array<uint32>(
    // TODO: Change to a memory pool we can somehow clear later.
    &memory->asset_memory_pool, n_indices, "mesh_indices"
  );

  for (uint32 idx_face = 0; idx_face < mesh_data->mNumFaces; idx_face++) {
    aiFace face = mesh_data->mFaces[idx_face];
    for (uint32 idx_index = 0; idx_index < face.mNumIndices; idx_index++) {
      mesh->indices.push(face.mIndices[idx_index]);
    }
  }
}


void ModelAsset::load_node(
  Memory *memory, aiNode *node, const aiScene *scene,
  glm::mat4 accumulated_transform, Pack indices_pack
) {
  glm::mat4 node_transform = Util::aimatrix4x4_to_glm(&node->mTransformation);
  glm::mat4 transform = accumulated_transform * node_transform;

  for (uint32 idx = 0; idx < node->mNumMeshes; idx++) {
    aiMesh *mesh_data = scene->mMeshes[node->mMeshes[idx]];
    load_mesh(
      memory,
      this->meshes.push(),
      mesh_data, scene,
      transform,
      indices_pack
    );
  }

  for (uint32 idx = 0; idx < node->mNumChildren; idx++) {
    Pack new_indices_pack = indices_pack;
    // NOTE: We can only store 4 bits per pack element. Our indices can be way bigger than
    // that, but that's fine. We don't need that much precision. Just smash the number up
    // if it's bigger.
    pack_push(&new_indices_pack, (uint8)idx);
    load_node(
      memory, node->mChildren[idx], scene, transform, new_indices_pack
    );
  }
}


void ModelAsset::load(Memory *memory) {
  // If we're not loading from a file, all the data has already
  // been loaded previously, so we just need to handle the
  // shaders and textures and so on, so skip this.
  this->mutex.lock();

  if (this->model_source == MODELSOURCE_FILE) {
    char path[256];
    strcpy(path, this->directory);
    strcat(path, "/");
    strcat(path, this->filename);

    const aiScene *scene = aiImportFile(
      path,
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

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
      log_fatal("assimp error: %s", aiGetErrorString());
      return;
    }

    load_node(
      memory, scene->mRootNode, scene, glm::mat4(1.0f), 0ULL
    );

    aiReleaseImport(scene);
  }

  this->is_mesh_data_loading_in_progress = false;
  this->is_mesh_data_loading_done = true;

  this->mutex.unlock();
}


void ModelAsset::copy_textures_to_pbo(PersistentPbo *persistent_pbo) {
  for (uint32 idx = 0; idx < this->mesh_templates.size; idx++) {
    MeshShaderTextureTemplate *mesh_template = this->mesh_templates.get(idx);

    if (mesh_template->texture_set) {
      mesh_template->texture_set->copy_textures_to_pbo(persistent_pbo);
    }
  }

  this->is_texture_copying_to_pbo_done = true;
  this->is_texture_copying_to_pbo_in_progress = false;
}


ModelAsset::ModelAsset(
  Memory *memory, ModelSource new_model_source,
  const char *new_name, const char *new_directory,
  const char *new_filename
) :
  meshes(&memory->asset_memory_pool, MAX_N_MESHES, "meshes"),
  texture_sets(&memory->asset_memory_pool, MAX_N_TEXTURE_SETS, "texture_sets"),
  mesh_templates(&memory->asset_memory_pool, MAX_N_MESH_TEMPLATES, "mesh_templates")
{
  this->name = new_name;
  this->model_source = new_model_source;
  this->directory = new_directory;
  this->filename = new_filename;

  // NOTE: We do not load MODELSOURCE_FILE models here.
  // They are loaded on-demand in `::draw()`.
}


ModelAsset::ModelAsset(
  Memory *memory, ModelSource new_model_source,
  real32 *vertex_data, uint32 n_vertices,
  uint32 *index_data, uint32 n_indices,
  const char *new_name,
  GLenum mode
) :
  meshes(&memory->asset_memory_pool, MAX_N_MESHES, "meshes"),
  texture_sets(&memory->asset_memory_pool, MAX_N_TEXTURE_SETS, "texture_sets"),
  mesh_templates(&memory->asset_memory_pool, MAX_N_MESH_TEMPLATES, "mesh_templates")
{
  this->name = new_name;
  this->model_source = new_model_source;
  this->directory = "";
  this->filename = "";

  Mesh *mesh = this->meshes.push();
  mesh->transform = glm::mat4(1.0f);
  mesh->texture_set = nullptr;
  mesh->mode = mode;
  mesh->n_vertices = n_vertices;
  mesh->n_indices = n_indices;

  setup_mesh_vertex_buffers_for_data_source(
    mesh, vertex_data, n_vertices, index_data, n_indices
  );
  this->is_mesh_data_loading_done = true;
  this->is_vertex_buffer_setup_done = true;
}


void ModelAsset::bind_texture_uniforms_for_mesh(Mesh *mesh) {
  TextureSet *texture_set = mesh->texture_set;
  ShaderAsset *shader_asset = mesh->shader_asset;

  if (!texture_set) {
    return;
  }

  if (shader_asset->type != SHADER_DEPTH) {
    glUseProgram(shader_asset->program);

    for (
      uint32 uniform_idx = 0;
      uniform_idx < shader_asset->n_intrinsic_uniforms;
      uniform_idx++
    ) {
      const char *uniform_name = shader_asset->intrinsic_uniform_names[uniform_idx];
      if (strcmp(uniform_name, "should_use_normal_map") == 0) {
        shader_asset->set_bool("should_use_normal_map", texture_set->should_use_normal_map);
      } else if (strcmp(uniform_name, "albedo_static") == 0) {
        shader_asset->set_vec4("albedo_static", &texture_set->albedo_static);
      } else if (strcmp(uniform_name, "metallic_static") == 0) {
        shader_asset->set_float("metallic_static", texture_set->metallic_static);
      } else if (strcmp(uniform_name, "roughness_static") == 0) {
        shader_asset->set_float("roughness_static", texture_set->roughness_static);
      } else if (strcmp(uniform_name, "ao_static") == 0) {
        shader_asset->set_float("ao_static", texture_set->ao_static);
      }
    }

    shader_asset->reset_texture_units();

    for (uint32 idx = 0; idx < texture_set->textures.size; idx++) {
      Texture *texture = texture_set->textures.get(idx);
      const char *uniform_name = *texture_set->texture_uniform_names.get(idx);
      log_info(
        "Setting uniforms: (model %s) (uniform_name %s) "
        "(texture->texture_name %d)",
        this->name, uniform_name, texture->texture_name
      );
      shader_asset->set_int(
        uniform_name,
        shader_asset->add_texture_unit(texture->texture_name, texture->target)
      );
    }
  }

  shader_asset->did_set_texture_uniforms = true;
}


void ModelAsset::set_shader_for_mesh(
  uint32 idx_mesh, ShaderAsset *shader_asset, ShaderAsset *depth_shader_asset
) {
  Mesh *mesh = this->meshes.get(idx_mesh);
  mesh->shader_asset = shader_asset;
  mesh->depth_shader_asset = depth_shader_asset;
}


void ModelAsset::set_shader(
  ShaderAsset *shader_asset, ShaderAsset *depth_shader_asset
) {
  for (uint32 idx_mesh = 0; idx_mesh < this->meshes.size; idx_mesh++) {
    set_shader_for_mesh(idx_mesh, shader_asset, depth_shader_asset);
  }
}


void ModelAsset::set_shader_for_node_idx(
  ShaderAsset *shader_asset, ShaderAsset *depth_shader_asset,
  uint8 node_depth, uint8 node_idx
) {
  for (uint32 idx_mesh = 0; idx_mesh < this->meshes.size; idx_mesh++) {
    Mesh *mesh = this->meshes.get(idx_mesh);
    if (pack_get(&mesh->indices_pack, node_depth) == node_idx) {
      set_shader_for_mesh(idx_mesh, shader_asset, depth_shader_asset);
    }
  }
}


void ModelAsset::bind_texture_to_mesh(
  uint32 idx_mesh, TextureSet *texture_set
) {
  Mesh *mesh = this->meshes.get(idx_mesh);
  mesh->texture_set = texture_set;
  if (!mesh->shader_asset->did_set_texture_uniforms) {
    bind_texture_uniforms_for_mesh(mesh);
  }
}


void ModelAsset::bind_texture(
  TextureSet *texture_set
) {
  for (uint32 idx_mesh = 0; idx_mesh < this->meshes.size; idx_mesh++) {
    bind_texture_to_mesh(idx_mesh, texture_set);
  }
}


void ModelAsset::bind_texture_for_node_idx(
  TextureSet *texture_set, uint8 node_depth, uint8 node_idx
) {
  for (uint32 idx_mesh = 0; idx_mesh < this->meshes.size; idx_mesh++) {
    Mesh *mesh = this->meshes.get(idx_mesh);
    if (pack_get(&mesh->indices_pack, node_depth) == node_idx) {
      bind_texture_to_mesh(idx_mesh, texture_set);
    }
  }
}


void ModelAsset::prepare_for_draw(
  Memory *memory,
  PersistentPbo *persistent_pbo,
  TextureNamePool *texture_name_pool,
  Queue<Task> *task_queue
) {
  // Step 1: Load mesh data. This is done on a separate thread.
  if (!this->is_mesh_data_loading_in_progress && !this->is_mesh_data_loading_done) {
    this->is_mesh_data_loading_in_progress = true;
    task_queue->push({TASKTYPE_LOAD_MODEL, this, nullptr, memory});
  }

  // Step 2: Once the mesh data is loaded, set up vertex buffers for these meshes.
  if (this->is_mesh_data_loading_done && !this->is_vertex_buffer_setup_done) {
    for (uint32 idx = 0; idx < this->meshes.size; idx++) {
      Mesh *mesh = this->meshes.get(idx);
      setup_mesh_vertex_buffers_for_file_source(mesh, &mesh->vertices, &mesh->indices);
    }
    this->is_vertex_buffer_setup_done = true;
  }

  if (this->mesh_templates.size > 0) {
    // Step 3: Once the mesh data is loaded, bind shaders for their respective meshes.
    if (this->is_mesh_data_loading_done && !this->is_shader_setting_done) {
      for (uint32 idx = 0; idx < this->mesh_templates.size; idx++) {
        MeshShaderTextureTemplate *mesh_template = this->mesh_templates.get(idx);

        if (mesh_template->apply_to_all_meshes) {
          set_shader(mesh_template->shader_asset, mesh_template->depth_shader_asset);
        } else {
          set_shader_for_node_idx(
            mesh_template->shader_asset,
            mesh_template->depth_shader_asset,
            mesh_template->node_depth, mesh_template->node_idx
          );
        }
      }
      this->is_shader_setting_done = true;
    }

    // Step 4: In parallel with the above, on a second thread, load all textures
    // for this model from their files and copy them over to the PBO.
    if (
      !this->is_texture_copying_to_pbo_done &&
      !this->is_texture_copying_to_pbo_in_progress
    ) {
      // NOTE: Make sure we don't spawn threads that do nothing, or queue up
      // useless work for threads! This means only copying textures for meshes
      // that have one or more textures that do not have names yet.
      bool32 should_try_to_copy_textures = false;
      for (uint32 idx = 0; idx < this->mesh_templates.size; idx++) {
        MeshShaderTextureTemplate *mesh_template = this->mesh_templates.get(idx);
        if (mesh_template->texture_set) {
          for (
            uint32 texture_idx = 0;
            texture_idx < mesh_template->texture_set->textures.size;
            texture_idx++
          ) {
            Texture *texture = mesh_template->texture_set->textures.get(texture_idx);
            if (!texture->texture_name) {
              should_try_to_copy_textures = true;
              break;
            }
          }
        }
      }

      if (should_try_to_copy_textures) {
        this->is_texture_copying_to_pbo_in_progress = true;
        task_queue->push({TASKTYPE_COPY_TEXTURES_TO_PBO, this, persistent_pbo, nullptr});
      } else {
        this->is_texture_copying_to_pbo_done = true;
        this->is_texture_creation_done = true;
      }
    }

    // Step 5: Once all of the above is complete, copy the texture data from the
    // PBO to the actual textures.
    if (
      this->is_mesh_data_loading_done &&
      this->is_vertex_buffer_setup_done &&
      this->is_shader_setting_done &&
      this->is_texture_copying_to_pbo_done &&
      !this->is_texture_creation_done
    ) {
      START_TIMER(copy_pbo_to_texture);

      bool32 did_have_to_generate_texture = false;

      for (uint32 idx = 0; idx < this->mesh_templates.size; idx++) {
        MeshShaderTextureTemplate *mesh_template = this->mesh_templates.get(idx);

        if (
          mesh_template->texture_set &&
          !mesh_template->texture_set->have_textures_been_generated
        ) {
          mesh_template->texture_set->generate_textures_from_pbo(
            persistent_pbo, texture_name_pool
          );
          did_have_to_generate_texture = true;
        }
      }

      if (!did_have_to_generate_texture) {
        this->is_texture_creation_done = true;
      }

      END_TIMER_MIN(copy_pbo_to_texture, 5);
    }

    // Step 6: Bind the texture uniforms.
    // NOTE: Because the shader might be reloaded at any time, we need to
    // check whether or not we need to set any uniforms every time.
    if (this->is_texture_creation_done) {
      for (uint32 idx = 0; idx < this->mesh_templates.size; idx++) {
        MeshShaderTextureTemplate *mesh_template = this->mesh_templates.get(idx);

        if (
          mesh_template->texture_set &&
          !mesh_template->shader_asset->did_set_texture_uniforms
        ) {
          if (mesh_template->apply_to_all_meshes) {
            bind_texture(
              mesh_template->texture_set
            );
          } else {
            bind_texture_for_node_idx(
              mesh_template->texture_set,
              mesh_template->node_depth, mesh_template->node_idx
            );
          }
        }
      }
    }
  }
}


void ModelAsset::draw(
  Memory *memory,
  PersistentPbo *persistent_pbo,
  TextureNamePool *texture_name_pool,
  Queue<Task> *task_queue,
  glm::mat4 *model_matrix,
  glm::mat3 *model_normal_matrix
) {
  prepare_for_draw(
    memory, persistent_pbo, texture_name_pool, task_queue
  );

  if (
    !this->is_mesh_data_loading_done ||
    !this->is_shader_setting_done ||
    !this->is_texture_creation_done
  ) {
    return;
  }

  uint32 last_used_shader_program = 0;
  ShaderAsset *shader_asset;

  for (uint32 mesh_idx = 0; mesh_idx < this->meshes.size; mesh_idx++) {
    Mesh *mesh = this->meshes.get(mesh_idx);
    shader_asset = mesh->shader_asset;

    // If our shader program has changed since our last mesh, tell OpenGL about it.
    bool32 has_shader_changed = false;
    if (shader_asset->program != last_used_shader_program) {
      has_shader_changed = true;
      glUseProgram(shader_asset->program);
      last_used_shader_program = shader_asset->program;

      for (
        uint32 texture_idx = 1;
        texture_idx < shader_asset->n_texture_units + 1; texture_idx++
      ) {
        if (shader_asset->texture_units[texture_idx] != 0) {
          glActiveTexture(GL_TEXTURE0 + texture_idx);
          glBindTexture(
            shader_asset->texture_unit_types[texture_idx],
            shader_asset->texture_units[texture_idx]
          );
        }
      }

      for (
        uint32 uniform_idx = 0;
        uniform_idx < shader_asset->n_intrinsic_uniforms;
        uniform_idx++
      ) {
        const char *uniform_name = shader_asset->intrinsic_uniform_names[uniform_idx];
        if (strcmp(uniform_name, "model_matrix") == 0) {
          shader_asset->set_mat4("model_matrix", model_matrix);
        } else if (strcmp(uniform_name, "model_normal_matrix") == 0) {
          shader_asset->set_mat3("model_normal_matrix", model_normal_matrix);
        }
      }
    }

#if 0
    if (strcmp(this->name, "ocean") == 0) {
      return;
    }
#endif

    glBindVertexArray(mesh->vao);
    if (mesh->n_indices > 0) {
      glDrawElements(mesh->mode, mesh->n_indices, GL_UNSIGNED_INT, 0);
    } else {
      glDrawArrays(mesh->mode, 0, mesh->n_vertices);
    }
  }
}


void ModelAsset::draw_in_depth_mode(
  Memory *memory,
  PersistentPbo *persistent_pbo,
  TextureNamePool *texture_name_pool,
  Queue<Task> *task_queue,
  glm::mat4 *model_matrix,
  glm::mat3 *model_normal_matrix,
  ShaderAsset *standard_depth_shader_asset
) {
  // TODO: We can probably merge this into the normal `draw()` above.
  prepare_for_draw(
    memory, persistent_pbo, texture_name_pool, task_queue
  );

  if (!this->is_mesh_data_loading_done || !this->is_shader_setting_done) {
    return;
  }

  uint32 last_used_shader_program = 0;
  ShaderAsset *shader_asset = standard_depth_shader_asset;

  for (uint32 idx = 0; idx < this->meshes.size; idx++) {
    Mesh *mesh = this->meshes.get(idx);
    if (mesh->depth_shader_asset) {
      shader_asset = mesh->depth_shader_asset;
    }

    // If our shader program has changed since our last mesh, tell OpenGL about it.
    bool32 has_shader_changed = false;
    if (shader_asset->program != last_used_shader_program) {
      glUseProgram(shader_asset->program);
      last_used_shader_program = shader_asset->program;
      has_shader_changed = true;

      for (
        uint32 uniform_idx = 0;
        uniform_idx < shader_asset->n_intrinsic_uniforms;
        uniform_idx++
      ) {
        const char *uniform_name = shader_asset->intrinsic_uniform_names[uniform_idx];
        if (strcmp(uniform_name, "model_matrix") == 0) {
          shader_asset->set_mat4("model_matrix", model_matrix);
        } else if (strcmp(uniform_name, "model_normal_matrix") == 0) {
          shader_asset->set_mat3("model_normal_matrix", model_normal_matrix);
        }
      }
    }

    glBindVertexArray(mesh->vao);
    if (mesh->n_indices > 0) {
      glDrawElements(mesh->mode, mesh->n_indices, GL_UNSIGNED_INT, 0);
    } else {
      glDrawArrays(mesh->mode, 0, mesh->n_vertices);
    }
  }
}


ModelAsset* ModelAsset::get_by_name(
  Array<ModelAsset> *assets, const char *name
) {
  for (uint32 idx = 0; idx < assets->size; idx++) {
    ModelAsset *asset = assets->get(idx);
    if (strcmp(asset->name, name) == 0) {
      return asset;
    }
  }
  log_warning("Could not find ModelAsset with name %s", name);
  return nullptr;
}
