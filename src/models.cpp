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
  mesh->texture_set = nullptr;
  mesh->mode = GL_TRIANGLES;

  mesh->indices_pack = indices_pack;

  // Vertices
  // NOTE: We're duplicating the vertex and index data because the format
  // assimp gives us the data in is not something we can directly use.
  // This is probably quite wasteful, but I haven't figure out a way to
  // elegantly use the data from assimp directly, and I don't think it's
  // possible.
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

  setup_mesh_vertex_buffers_for_file_source(mesh, &vertices, &indices);
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
  memory->temp_memory_pool.reset();

  this->is_loaded = true;
}


ModelAsset::ModelAsset(
  Memory *memory, ModelSource new_model_source,
  const char *new_name, const char *new_directory,
  const char *new_filename
) : meshes(&memory->asset_memory_pool, MAX_N_MESHES, "meshes"),
  texture_sets(&memory->asset_memory_pool, MAX_N_TEXTURE_SETS, "texture_sets")
{
  this->name = new_name;
  this->is_loaded = false;
  this->model_source = new_model_source;
  this->directory = new_directory;
  this->filename = new_filename;

  // NOTE: We do not load MODELSOURCE_FILE models here.
  // They are loaded on-demand in `::draw()`.
  load(memory);
}


ModelAsset::ModelAsset(
  Memory *memory, ModelSource new_model_source,
  real32 *vertex_data, uint32 n_vertices,
  uint32 *index_data, uint32 n_indices,
  const char *new_name,
  GLenum mode
) : meshes(&memory->asset_memory_pool, MAX_N_MESHES, "meshes"),
  texture_sets(&memory->asset_memory_pool, MAX_N_TEXTURE_SETS, "texture_sets")
{
  this->name = new_name;
  this->is_loaded = true;
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

  memory->temp_memory_pool.reset();
}


void ModelAsset::init_texture_set(TextureSet *texture_set, uint32 id) {
  texture_set->id = id;

  texture_set->albedo_texture = 0;
  texture_set->metallic_texture = 0;
  texture_set->roughness_texture = 0;
  texture_set->ao_texture = 0;
  texture_set->normal_texture = 0;

  texture_set->albedo_static = glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f);
  texture_set->metallic_static = -1.0f;
  texture_set->roughness_static = -1.0f;
  texture_set->ao_static = -1.0f;
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
  } else {
    log_info(
      "Tried to set texture uniforms, but there is nothing to do for this shader: \"%s\"",
      shader_asset->name
    );
  }

  shader_asset->did_set_texture_uniforms = true;
}


void ModelAsset::bind_as_screenquad(
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


void ModelAsset::draw(Memory *memory, glm::mat4 *model_matrix) {
  if (!this->is_loaded) {
    log_info("Lazily loading model: %s", this->name);
    load(memory);
  }

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
  Memory *memory,
  glm::mat4 *model_matrix, ShaderAsset *entity_depth_shader_asset
) {
  if (!this->is_loaded) {
    log_info("Lazily loading model: %s", this->name);
    load(memory);
  }

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
