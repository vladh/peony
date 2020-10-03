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
    GL_ARRAY_BUFFER, sizeof(Vertex) * vertices->get_size(),
    vertices->get_items_ptr(), GL_STATIC_DRAW
  );

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * indices->get_size(),
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
  mesh->texture_set_asset = nullptr;
  mesh->mode = GL_TRIANGLES;

  mesh->indices_pack = indices_pack;

  // Vertices
  // NOTE: We're duplicating the vertex and index data because the format
  // assimp gives us the data in is not something we can directly use.
  // This is probably quite wasteful, but I haven't figured out a way to
  // elegantly use the data from assimp directly, and I don't think it's
  // possible.
  mesh->vertices = Array<Vertex>(
    &memory->temp_memory_pool, mesh_data->mNumVertices, "mesh_vertices"
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
    vertex->position = position;

    glm::vec3 normal;
    normal.x = mesh_data->mNormals[idx].x;
    normal.y = mesh_data->mNormals[idx].y;
    normal.z = mesh_data->mNormals[idx].z;
    vertex->normal = normal;

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
    &memory->temp_memory_pool, n_indices, "mesh_indices"
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
  if (this->model_source == MODELSOURCE_FILE) {
    char path[256];
    strcpy(path, this->directory);
    strcat(path, "/");
    strcat(path, this->filename);

    this->mutex.lock();
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
      // NOTE: Uncomment this when changing to proper normal mapping.
      /* | aiProcess_CalcTangentSpace */
    );
    this->mutex.unlock();

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
}


void ModelAsset::preload_texture_set_image_data() {
  for (uint32 idx = 0; idx < this->mesh_templates.get_size(); idx++) {
    MeshShaderTextureTemplate *mesh_template = this->mesh_templates.get(idx);

    if (
      mesh_template->texture_set_asset &&
      !mesh_template->texture_set_asset->is_loading_done &&
      !mesh_template->texture_set_asset->is_image_data_preloaded
    ) {
      mesh_template->texture_set_asset->preload_image_data();
    }
  }

  this->is_texture_preload_done = true;
  this->is_texture_preload_in_progress = false;
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
) : meshes(&memory->asset_memory_pool, MAX_N_MESHES, "meshes"),
  texture_sets(&memory->asset_memory_pool, MAX_N_TEXTURE_SETS, "texture_sets"),
  mesh_templates(&memory->asset_memory_pool, MAX_N_MESH_TEMPLATES, "mesh_templates")
{
  this->name = new_name;
  this->is_mesh_data_loading_done = true;
  this->model_source = new_model_source;
  this->directory = "";
  this->filename = "";

  Mesh *mesh = this->meshes.push();
  mesh->transform = glm::mat4(1.0f);
  mesh->texture_set_asset = nullptr;
  mesh->mode = mode;
  mesh->n_vertices = n_vertices;
  mesh->n_indices = n_indices;

  setup_mesh_vertex_buffers_for_data_source(
    mesh, vertex_data, n_vertices, index_data, n_indices
  );
  this->is_vertex_buffer_setup_done = true;
}


void ModelAsset::bind_texture_uniforms_for_mesh(Mesh *mesh) {
  TextureSetAsset *texture_set_asset = mesh->texture_set_asset;
  ShaderAsset *shader_asset = mesh->shader_asset;

  // NOTE: We set this for every mesh, but we only need to run this for every
  // shaders, and most meshes share a shader, so we're kind of being wasteful.
  // It's not a big deal, but could we improve this?
  if (shader_asset->did_set_texture_uniforms || !texture_set_asset) {
    return;
  }

  if (shader_asset->type == SHADER_STANDARD) {
    glUseProgram(shader_asset->program);

    bool should_use_normal_map = texture_set_asset->normal_texture != 0;
    shader_asset->set_bool("should_use_normal_map", should_use_normal_map);

    shader_asset->set_vec4("albedo_static", &texture_set_asset->albedo_static);
    shader_asset->set_float("metallic_static", texture_set_asset->metallic_static);
    shader_asset->set_float("roughness_static", texture_set_asset->roughness_static);
    shader_asset->set_float("ao_static", texture_set_asset->ao_static);

    shader_asset->set_int(
      "albedo_texture",
      shader_asset->add_texture_unit(texture_set_asset->albedo_texture, GL_TEXTURE_2D)
    );

    shader_asset->set_int(
      "metallic_texture",
      shader_asset->add_texture_unit(texture_set_asset->metallic_texture, GL_TEXTURE_2D)
    );

    shader_asset->set_int(
      "roughness_texture",
      shader_asset->add_texture_unit(texture_set_asset->roughness_texture, GL_TEXTURE_2D)
    );

    shader_asset->set_int(
      "ao_texture",
      shader_asset->add_texture_unit(texture_set_asset->ao_texture, GL_TEXTURE_2D)
    );

    shader_asset->set_int(
      "normal_texture",
      shader_asset->add_texture_unit(texture_set_asset->normal_texture, GL_TEXTURE_2D)
    );
  } else {
    log_info(
      "Tried to set texture uniforms, but there is nothing to do for this shader: \"%s\"",
      shader_asset->name
    );
  }

  shader_asset->did_set_texture_uniforms = true;
}


void ModelAsset::bind_shader_and_texture_as_screenquad(
  uint32 g_position_texture, uint32 g_normal_texture,
  uint32 g_albedo_texture, uint32 g_pbr_texture,
  uint32 n_depth_textures,
  uint32 *depth_textures,
  ShaderAsset *shader_asset
) {
  Mesh *mesh = this->meshes.get(0);
  mesh->shader_asset = shader_asset;

  glUseProgram(shader_asset->program);

  shader_asset->set_int("n_depth_textures", n_depth_textures);

  for (uint32 idx = 0; idx < MAX_N_SHADOW_FRAMEBUFFERS; idx++) {
    shader_asset->set_int(
      DEPTH_TEXTURE_UNIFORM_NAMES[idx],
      shader_asset->add_texture_unit(depth_textures[idx], GL_TEXTURE_CUBE_MAP)
    );
  }

  shader_asset->set_int(
    "g_position_texture",
    shader_asset->add_texture_unit(g_position_texture, GL_TEXTURE_2D)
  );

  shader_asset->set_int(
    "g_normal_texture",
    shader_asset->add_texture_unit(g_normal_texture, GL_TEXTURE_2D)
  );

  shader_asset->set_int(
    "g_albedo_texture",
    shader_asset->add_texture_unit(g_albedo_texture, GL_TEXTURE_2D)
  );

  shader_asset->set_int(
    "g_pbr_texture",
    shader_asset->add_texture_unit(g_pbr_texture, GL_TEXTURE_2D)
  );

  shader_asset->did_set_texture_uniforms = true;
}


void ModelAsset::set_shader_to_mesh(
  uint32 idx_mesh, ShaderAsset *shader_asset
) {
  Mesh *mesh = this->meshes.get(idx_mesh);
  mesh->shader_asset = shader_asset;
}


void ModelAsset::set_shader(
  ShaderAsset *shader_asset
) {
  for (uint32 idx_mesh = 0; idx_mesh < this->meshes.get_size(); idx_mesh++) {
    set_shader_to_mesh(idx_mesh, shader_asset);
  }
}


void ModelAsset::set_shader_for_node_idx(
  ShaderAsset *shader_asset, uint8 node_depth, uint8 node_idx
) {
  for (uint32 idx_mesh = 0; idx_mesh < this->meshes.get_size(); idx_mesh++) {
    Mesh *mesh = this->meshes.get(idx_mesh);
    if (pack_get(&mesh->indices_pack, node_depth) == node_idx) {
      set_shader_to_mesh(idx_mesh, shader_asset);
    }
  }
}


void ModelAsset::bind_texture_to_mesh(
  uint32 idx_mesh, TextureSetAsset *texture_set_asset
) {
  Mesh *mesh = this->meshes.get(idx_mesh);
  mesh->texture_set_asset = texture_set_asset;
  bind_texture_uniforms_for_mesh(mesh);
}


void ModelAsset::bind_texture(
  TextureSetAsset *texture_set_asset
) {
  for (uint32 idx_mesh = 0; idx_mesh < this->meshes.get_size(); idx_mesh++) {
    bind_texture_to_mesh(idx_mesh, texture_set_asset);
  }
}


void ModelAsset::bind_texture_for_node_idx(
  TextureSetAsset *texture_set_asset, uint8 node_depth, uint8 node_idx
) {
  for (uint32 idx_mesh = 0; idx_mesh < this->meshes.get_size(); idx_mesh++) {
    Mesh *mesh = this->meshes.get(idx_mesh);
    if (pack_get(&mesh->indices_pack, node_depth) == node_idx) {
      bind_texture_to_mesh(idx_mesh, texture_set_asset);
    }
  }
}


void ModelAsset::prepare_for_draw(Memory *memory) {
  // TODO: Update comment.
  // NOTE: Before we draw, we have to do 4 things.
  //
  // 1. Load the model. This is done on a separate thread. If the model is
  //   already being loaded, don't try to load it again. Loading the model
  //   will store its vertices and indices in memory.
  // 2. Load the images for the textures. This is done on a separate
  //   thread.
  //
  // After the model data and textures are loaded, we have the meshes, and we
  // can do the following, which must be done on the main (OpenGL) thread:
  //
  // 3. Set up vertex buffers for each mesh.
  // 4. Set the shaders for each mesh.
  // 5. Create textures from the images and bind their uniforms for each mesh.

  if (!this->is_mesh_data_loading_in_progress && !this->is_mesh_data_loading_done) {
    log_info("%s: STEP 1 - Loading mesh data", this->name);
    this->is_mesh_data_loading_in_progress = true;
    *global_threads.push() = std::thread(&ModelAsset::load, this, memory);
    /* load(memory); */
  }

  if (this->is_mesh_data_loading_done && !this->is_vertex_buffer_setup_done) {
    log_info("%s: STEP 2 - Setting up vertex buffers", this->name);
    for (uint32 idx = 0; idx < this->meshes.get_size(); idx++) {
      Mesh *mesh = this->meshes.get(idx);
      setup_mesh_vertex_buffers_for_file_source(mesh, &mesh->vertices, &mesh->indices);
    }
    this->is_vertex_buffer_setup_done = true;
    log_info("...done");
  }

  if (this->is_mesh_data_loading_done && !this->is_shader_setting_done) {
    log_info("%s: STEP 3 - Binding shaders", this->name);
    for (uint32 idx = 0; idx < this->mesh_templates.get_size(); idx++) {
      MeshShaderTextureTemplate *mesh_template = this->mesh_templates.get(idx);

      if (mesh_template->apply_to_all_meshes) {
        set_shader(mesh_template->shader_asset);
      } else {
        set_shader_for_node_idx(
          mesh_template->shader_asset,
          mesh_template->node_depth, mesh_template->node_idx
        );
      }
    }
    this->is_shader_setting_done = true;
    log_info("...done");
  }

  if (!this->is_texture_preload_in_progress && !this->is_texture_preload_done) {
    log_info("%s: STEP 4 - Preloading textures", this->name);
    this->is_texture_preload_in_progress = true;
    *global_threads.push() = std::thread(&ModelAsset::preload_texture_set_image_data, this);
    /* preload_texture_set_image_data(); */
  }

  if (
    this->is_mesh_data_loading_done &&
    this->is_vertex_buffer_setup_done &&
    this->is_shader_setting_done &&
    this->is_texture_preload_done &&
    !this->is_texture_set_binding_done
  ) {
    log_info("%s: STEP 5 - Loading textures", this->name);
    for (uint32 idx = 0; idx < this->mesh_templates.get_size(); idx++) {
      MeshShaderTextureTemplate *mesh_template = this->mesh_templates.get(idx);

      if (mesh_template->texture_set_asset) {
        mesh_template->texture_set_asset->load_textures_from_preloaded_data();

        if (mesh_template->apply_to_all_meshes) {
          bind_texture(
            mesh_template->texture_set_asset
          );
        } else {
          bind_texture_for_node_idx(
            mesh_template->texture_set_asset,
            mesh_template->node_depth, mesh_template->node_idx
          );
        }
      }
    }
    this->is_texture_set_binding_done = true;
    log_info("...done");
  }
}


void ModelAsset::draw(Memory *memory, glm::mat4 *model_matrix) {
  prepare_for_draw(memory);
  if (!this->is_mesh_data_loading_done || !this->is_shader_setting_done) {
    return;
  }

  uint32 last_used_shader_program = 0;

  for (uint32 idx = 0; idx < this->meshes.get_size(); idx++) {
    Mesh *mesh = this->meshes.get(idx);
    ShaderAsset *shader_asset = mesh->shader_asset;

    // If our shader program has changed since our last mesh, tell OpenGL about it.
    if (shader_asset->program != last_used_shader_program) {
      glUseProgram(shader_asset->program);
      last_used_shader_program = shader_asset->program;

      if (shader_asset->type == SHADER_STANDARD || shader_asset->type == SHADER_OTHER_OBJECT) {
        shader_asset->set_mat4("model", model_matrix);
      }

      for (
        uint32 texture_idx = 1; texture_idx < shader_asset->n_texture_units + 1; texture_idx++
      ) {
        if (shader_asset->texture_units[texture_idx] != 0) {
          glActiveTexture(GL_TEXTURE0 + texture_idx);
          glBindTexture(
            shader_asset->texture_unit_types[texture_idx],
            shader_asset->texture_units[texture_idx]
          );
        }
      }
    }

    if (shader_asset->type == SHADER_STANDARD) {
      shader_asset->set_mat4("mesh_transform", &mesh->transform);
    }

    glBindVertexArray(mesh->vao);
    if (mesh->n_indices > 0) {
      glDrawElements(mesh->mode, mesh->n_indices, GL_UNSIGNED_INT, 0);
    } else {
      glDrawArrays(mesh->mode, 0, mesh->n_vertices);
    }
    glBindVertexArray(0);
  }
}


void ModelAsset::draw_in_depth_mode(
  Memory *memory,
  glm::mat4 *model_matrix, ShaderAsset *standard_depth_shader_asset
) {
  prepare_for_draw(memory);
  if (!this->is_mesh_data_loading_done || !this->is_shader_setting_done) {
    return;
  }

  ShaderAsset *shader_asset = standard_depth_shader_asset;

  glUseProgram(shader_asset->program);
  shader_asset->set_mat4("model", model_matrix);

  for (uint32 idx = 0; idx < this->meshes.get_size(); idx++) {
    Mesh *mesh = this->meshes.get(idx);
    shader_asset->set_mat4("mesh_transform", &mesh->transform);

    glBindVertexArray(mesh->vao);
    if (mesh->n_indices > 0) {
      glDrawElements(mesh->mode, mesh->n_indices, GL_UNSIGNED_INT, 0);
    } else {
      glDrawArrays(mesh->mode, 0, mesh->n_vertices);
    }
    glBindVertexArray(0);
  }
}


ModelAsset* ModelAsset::get_by_name(
  Array<ModelAsset> *assets, const char *name
) {
  for (uint32 idx = 0; idx < assets->get_size(); idx++) {
    ModelAsset *asset = assets->get(idx);
    if (strcmp(asset->name, name) == 0) {
      return asset;
    }
  }
  log_warning("Could not find ModelAsset with name %s", name);
  return nullptr;
}
