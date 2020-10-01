void ModelAsset::setup_mesh_vertex_buffers(
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
  mesh->texture_set = nullptr;
  mesh->mode = GL_TRIANGLES;

  mesh->indices_pack = indices_pack;

  // Vertices
  // NOTE: This is probably #slow. We're copying data around from one format
  // to another for no real reason. There would probably be a smart way to
  // use the original data, but it's not a big bottleneck according to the
  // profiler.
  Array<Vertex> vertices(
    &memory->temp_memory_pool, mesh_data->mNumVertices, "mesh_vertices"
  );

  if (!mesh_data->mNormals) {
    log_warning("Model does not have normals.");
  }

  mesh->n_vertices = mesh_data->mNumVertices;

  for (uint32 idx = 0; idx < mesh_data->mNumVertices; idx++) {
    Vertex *vertex = vertices.push();

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
  Array<uint32> indices(
    &memory->temp_memory_pool, n_indices, "mesh_indices"
  );

  for (uint32 idx_face = 0; idx_face < mesh_data->mNumFaces; idx_face++) {
    aiFace face = mesh_data->mFaces[idx_face];
    for (uint32 idx_index = 0; idx_index < face.mNumIndices; idx_index++) {
      indices.push(face.mIndices[idx_index]);
    }
  }

  setup_mesh_vertex_buffers(mesh, &vertices, &indices);
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


void ModelAsset::load_model(Memory *memory) {
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
    // NOTE: Uncomment this when changing to proper normal mapping.
    /* | aiProcess_CalcTangentSpace */
  );

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    log_error("assimp error: %s", aiGetErrorString());
    return;
  }

  load_node(
    memory, scene->mRootNode, scene, glm::mat4(1.0f), 0ULL
  );

  aiReleaseImport(scene);
}


ModelAsset::ModelAsset(
  Memory *memory, const char *new_name, const char *new_directory,
  const char *new_filename
) : meshes(&memory->asset_memory_pool, MAX_N_MESHES, "meshes"),
  texture_sets(&memory->asset_memory_pool, MAX_N_TEXTURE_SETS, "texture_sets")
{
  this->name = new_name;
  this->directory = new_directory;
  this->filename = new_filename;

  load_model(memory);
  memory->temp_memory_pool.reset();
}


ModelAsset::ModelAsset(
  Memory *memory,
  real32 *vertex_data, uint32 n_vertices,
  uint32 *index_data, uint32 n_indices,
  const char *new_name,
  GLenum mode
) : meshes(&memory->asset_memory_pool, MAX_N_MESHES, "meshes"),
  texture_sets(&memory->asset_memory_pool, MAX_N_TEXTURE_SETS, "texture_sets")
{
  this->name = new_name;
  this->directory = "";
  this->filename = "";

  Mesh *mesh = this->meshes.push();
  meshes.get(0)->transform = glm::mat4(1.0f);
  meshes.get(0)->texture_set = nullptr;
  meshes.get(0)->mode = mode;

  // NOTE: We are copying this data around for no real reason.
  // It probably doesn't matter as this is pretty much debug code,
  // but it might be good to improve it.

  // Vertices
  Array<Vertex> vertices = Array<Vertex>(
    &memory->temp_memory_pool, n_vertices, "mesh_vertices"
  );
  mesh->n_vertices = n_vertices;

  for (uint32 idx = 0; idx < n_vertices; idx++) {
    Vertex *vertex = vertices.push();

    glm::vec3 position;
    position.x = vertex_data[(idx * 8) + 0];
    position.y = vertex_data[(idx * 8) + 1];
    position.z = vertex_data[(idx * 8) + 2];
    vertex->position = position;

    glm::vec3 normal;
    normal.x = vertex_data[(idx * 8) + 3];
    normal.y = vertex_data[(idx * 8) + 4];
    normal.z = vertex_data[(idx * 8) + 5];
    vertex->normal = normal;

    glm::vec2 tex_coords;
    tex_coords.x = vertex_data[(idx * 8) + 6];
    tex_coords.y = vertex_data[(idx * 8) + 7];
    vertex->tex_coords = tex_coords;
  }

  // Indices
  Array<uint32> indices = Array<uint32>(
    &memory->temp_memory_pool, n_indices, n_indices, index_data
  );
  mesh->n_indices = n_indices;

  setup_mesh_vertex_buffers(mesh, &vertices, &indices);

  memory->temp_memory_pool.reset();
}


void ModelAsset::init_texture_set(TextureSet *texture_set, uint32 id) {
  texture_set->id = id;

  texture_set->n_depth_textures = 0;

  texture_set->albedo_texture = 0;
  texture_set->metallic_texture = 0;
  texture_set->roughness_texture = 0;
  texture_set->ao_texture = 0;
  texture_set->normal_texture = 0;

  texture_set->albedo_static = glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f);
  texture_set->metallic_static = -1.0f;
  texture_set->roughness_static = -1.0f;
  texture_set->ao_static = -1.0f;

  texture_set->g_position_texture = 0;
  texture_set->g_normal_texture = 0;
  texture_set->g_albedo_texture = 0;
  texture_set->g_pbr_texture = 0;
}


TextureSet* ModelAsset::create_texture_set() {
  // NOTE: As an ID for the texture set, we take its index in the
  // texture set array, plus one. This means that valid indices start from
  // 1 on. This is so we can use 0 as an "unset" value. We might want to do
  // something else in the future, but this should do.
  /* uint32 texture_set_id = texture_sets.size + 1; */

  // We initialise texture_set ids to something random and hopefully
  // more or less globally unique.
  // TODO: Change this to something actually less likely to collide.
  uint32 texture_set_id = (uint32)Util::random(0, UINT32_MAX);

  TextureSet *texture_set = this->texture_sets.push();

  init_texture_set(texture_set, texture_set_id);
  return texture_set;
}


void ModelAsset::bind_texture_uniforms_for_mesh(Mesh *mesh) {
  TextureSet *texture_set = mesh->texture_set;
  ShaderAsset *shader_asset = mesh->shader_asset;

  if (shader_asset->did_set_texture_uniforms || !texture_set) {
    return;
  }

  if (shader_asset->type == SHADER_ENTITY) {
    glUseProgram(shader_asset->program);

    bool should_use_normal_map = texture_set->normal_texture != 0;
    shader_asset->set_bool("should_use_normal_map", should_use_normal_map);

    shader_asset->set_vec4("albedo_static", &texture_set->albedo_static);
    shader_asset->set_float("metallic_static", texture_set->metallic_static);
    shader_asset->set_float("roughness_static", texture_set->roughness_static);
    shader_asset->set_float("ao_static", texture_set->ao_static);

    shader_asset->set_int(
      "albedo_texture",
      shader_asset->add_texture_unit(texture_set->albedo_texture, GL_TEXTURE_2D)
    );

    shader_asset->set_int(
      "metallic_texture",
      shader_asset->add_texture_unit(texture_set->metallic_texture, GL_TEXTURE_2D)
    );

    shader_asset->set_int(
      "roughness_texture",
      shader_asset->add_texture_unit(texture_set->roughness_texture, GL_TEXTURE_2D)
    );

    shader_asset->set_int(
      "ao_texture",
      shader_asset->add_texture_unit(texture_set->ao_texture, GL_TEXTURE_2D)
    );

    shader_asset->set_int(
      "normal_texture",
      shader_asset->add_texture_unit(texture_set->normal_texture, GL_TEXTURE_2D)
    );
  } else if (shader_asset->type == SHADER_LIGHTING) {
    glUseProgram(shader_asset->program);

    shader_asset->set_int("n_depth_textures", texture_set->n_depth_textures);

    for (uint32 idx = 0; idx < MAX_N_SHADOW_FRAMEBUFFERS; idx++) {
      shader_asset->set_int(
        DEPTH_TEXTURE_UNIFORM_NAMES[idx],
        shader_asset->add_texture_unit(texture_set->depth_textures[idx], GL_TEXTURE_CUBE_MAP)
      );
    }

    shader_asset->set_int(
      "g_position_texture",
      shader_asset->add_texture_unit(texture_set->g_position_texture, GL_TEXTURE_2D)
    );

    shader_asset->set_int(
      "g_normal_texture",
      shader_asset->add_texture_unit(texture_set->g_normal_texture, GL_TEXTURE_2D)
    );

    shader_asset->set_int(
      "g_albedo_texture",
      shader_asset->add_texture_unit(texture_set->g_albedo_texture, GL_TEXTURE_2D)
    );

    shader_asset->set_int(
      "g_pbr_texture",
      shader_asset->add_texture_unit(texture_set->g_pbr_texture, GL_TEXTURE_2D)
    );
  } else {
    log_info(
      "Tried to set texture uniforms, but there is nothing to do for this shader: \"%s\"",
      shader_asset->name
    );
  }

  shader_asset->did_set_texture_uniforms = true;
}


void ModelAsset::set_shader_asset_for_mesh(
  uint32 idx_mesh, ShaderAsset *shader_asset
) {
  Mesh *mesh = this->meshes.get(idx_mesh);
  mesh->shader_asset = shader_asset;
  bind_texture_uniforms_for_mesh(mesh);
}


void ModelAsset::set_shader_asset(ShaderAsset *shader_asset) {
  for (uint32 idx_mesh = 0; idx_mesh < this->meshes.get_size(); idx_mesh++) {
    set_shader_asset_for_mesh(idx_mesh, shader_asset);
  }
}


void ModelAsset::set_shader_asset_for_node_idx(
  ShaderAsset *shader_asset, uint8 node_depth, uint8 node_idx
) {
  for (uint32 idx_mesh = 0; idx_mesh < this->meshes.get_size(); idx_mesh++) {
    Mesh *mesh = this->meshes.get(idx_mesh);
    if (pack_get(&mesh->indices_pack, node_depth) == node_idx) {
      set_shader_asset_for_mesh(idx_mesh, shader_asset);
    }
  }
}


void ModelAsset::bind_texture_set_to_mesh(TextureSet *texture_set) {
  for (uint32 idx_mesh = 0; idx_mesh < this->meshes.get_size(); idx_mesh++) {
    Mesh *mesh = this->meshes.get(idx_mesh);
    mesh->texture_set = texture_set;
  }
}


void ModelAsset::bind_texture_set_to_mesh_for_node_idx(
  TextureSet *texture_set, uint8 node_depth, uint8 node_idx
) {
  for (uint32 idx_mesh = 0; idx_mesh < this->meshes.get_size(); idx_mesh++) {
    Mesh *mesh = this->meshes.get(idx_mesh);
    if (pack_get(&mesh->indices_pack, node_depth) == node_idx) {
      mesh->texture_set = texture_set;
    }
  }
}


void ModelAsset::draw(glm::mat4 *model_matrix) {
  uint32 last_used_texture_set_id = 0;
  uint32 last_used_shader_program = 0;

  for (uint32 idx = 0; idx < this->meshes.get_size(); idx++) {
    Mesh *mesh = this->meshes.get(idx);
    ShaderAsset *shader_asset = mesh->shader_asset;

    // If our shader program has changed since our last mesh, tell OpenGL about it.
    if (shader_asset->program != last_used_shader_program) {
      glUseProgram(shader_asset->program);
      last_used_shader_program = shader_asset->program;

      if (shader_asset->type == SHADER_ENTITY || shader_asset->type == SHADER_OTHER_OBJECT) {
        shader_asset->set_mat4("model", model_matrix);
      }
    }

    // If our texture set has changed since our last mesh, tell OpenGL about it.
    if (mesh->texture_set && (mesh->texture_set->id != last_used_texture_set_id)) {
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
      last_used_texture_set_id = mesh->texture_set->id;
    }

    if (shader_asset->type == SHADER_ENTITY) {
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
  glm::mat4 *model_matrix, ShaderAsset *entity_depth_shader_asset
) {
  ShaderAsset *shader_asset = entity_depth_shader_asset;

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
