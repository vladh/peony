uint32 models_load_texture_from_file(const char *path) {
  uint32 texture_id;
  glGenTextures(1, &texture_id);

  int32 width, height, n_components;
  unsigned char *data = util_load_image(
    path, &width, &height, &n_components
  );

  if (data) {
    GLenum format = GL_RGB;
    if (n_components == 1) {
      format = GL_RED;
    } else if (n_components == 3) {
      format = GL_RGB;
    } else if (n_components == 4) {
      format = GL_RGBA;
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    util_free_image(data);
  } else {
    log_error("Texture failed to load at path: %s", path);
    util_free_image(data);
  }

  return texture_id;
}

void models_setup_mesh(Mesh *mesh) {
  glGenVertexArrays(1, &mesh->vao);
  glGenBuffers(1, &mesh->vbo);
  glGenBuffers(1, &mesh->ebo);

  glBindVertexArray(mesh->vao);

  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
  glBufferData(
    GL_ARRAY_BUFFER, sizeof(Vertex) * mesh->vertices.size,
    mesh->vertices.items, GL_STATIC_DRAW
  );

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * mesh->indices.size,
    mesh->indices.items, GL_STATIC_DRAW
  );

  // TODO: Somehow fix these janky locations.
  uint32 location;

  /* location = glGetAttribLocation(shader_program, "position"); */
  location = 0;
  glEnableVertexAttribArray(location);
  glVertexAttribPointer(
    location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0
  );

  /* location = glGetAttribLocation(shader_program, "normal"); */
  location = 1;
  glEnableVertexAttribArray(location);
  glVertexAttribPointer(
    location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)
  );

  /* location = glGetAttribLocation(shader_program, "tex_coords"); */
  location = 2;
  glEnableVertexAttribArray(location);
  glVertexAttribPointer(
    location, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords)
  );
}

internal void models_load_mesh_vertices(
  Memory *memory, Model *model,
  Mesh *mesh, aiMesh *mesh_data, const aiScene *scene
) {
  mesh->vertices.size = 0;
  mesh->vertices.max_size = mesh_data->mNumVertices;
  mesh->vertices.items = (Vertex*)memory_push_memory_to_pool(
    &memory->temp_memory_pool, sizeof(Vertex) * mesh->vertices.max_size,
    "vertices"
  );

  if (!mesh_data->mNormals) {
    log_warning("Model does not have normals.");
  }

  for (uint32 idx = 0; idx < mesh_data->mNumVertices; idx++) {
    Vertex *vertex = (Vertex*)array_push<Vertex>(&mesh->vertices);

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
      tex_coords.y = mesh_data->mTextureCoords[0][idx].y;
      vertex->tex_coords = tex_coords;
    } else {
      vertex->tex_coords = glm::vec2(0.0f, 0.0f);
    }
  }
}

internal void models_load_mesh_indices(
  Memory *memory, Model *model,
  Mesh *mesh, aiMesh *mesh_data, const aiScene *scene
) {
  uint32 n_indices = 0;
  for (uint32 idx_face = 0; idx_face < mesh_data->mNumFaces; idx_face++) {
    aiFace face = mesh_data->mFaces[idx_face];
    n_indices += face.mNumIndices;
  }

  mesh->does_use_indices = (n_indices > 0);

  mesh->indices.size = 0;
  mesh->indices.max_size = n_indices;
  mesh->indices.items = (uint32*)memory_push_memory_to_pool(
    &memory->temp_memory_pool, sizeof(uint32) * mesh->indices.max_size,
    "indices"
  );

  for (uint32 idx_face = 0; idx_face < mesh_data->mNumFaces; idx_face++) {
    aiFace face = mesh_data->mFaces[idx_face];
    for (uint32 idx_index = 0; idx_index < face.mNumIndices; idx_index++) {
      array_push(&mesh->indices, face.mIndices[idx_index]);
    }
  }
}

internal void models_load_mesh_textures(
  Memory *memory, Model *model,
  Mesh *mesh, aiMesh *mesh_data, const aiScene *scene
) {
  aiString texture_filename_aistring;
  char texture_path[128];
  aiMaterial *material = scene->mMaterials[mesh_data->mMaterialIndex];

  mesh->n_diffuse_textures = material->GetTextureCount(aiTextureType_DIFFUSE);
  mesh->n_specular_textures = material->GetTextureCount(aiTextureType_SPECULAR);
  mesh->n_depth_textures = 0;

  for (uint32 idx = 0; idx < mesh->n_diffuse_textures; idx++) {
    material->GetTexture(aiTextureType_DIFFUSE, idx, &texture_filename_aistring);
    sprintf(texture_path, "%s/%s", model->directory, texture_filename_aistring.C_Str());
    mesh->diffuse_textures[idx] = models_load_texture_from_file(
      texture_path
    );
  }

  for (uint32 idx = 0; idx < mesh->n_specular_textures; idx++) {
    material->GetTexture(aiTextureType_SPECULAR, idx, &texture_filename_aistring);
    sprintf(texture_path, "%s/%s", model->directory, texture_filename_aistring.C_Str());
    mesh->specular_textures[idx] = models_load_texture_from_file(
      texture_path
    );
  }
}

void models_load_mesh(
  Memory *memory, Model *model,
  Mesh *mesh, aiMesh *mesh_data, const aiScene *scene
) {
  mesh->mode = GL_TRIANGLES;
  models_load_mesh_vertices(
    memory, model, mesh, mesh_data, scene
  );
  models_load_mesh_indices(
    memory, model, mesh, mesh_data, scene
  );
  if (model->should_load_textures_from_file) {
    models_load_mesh_textures(
      memory, model, mesh, mesh_data, scene
    );
  }
  models_setup_mesh(mesh);
}

void models_load_model_node(
  Memory *memory, Model *model,
  aiNode *node, const aiScene *scene
) {
  for (uint32 idx = 0; idx < node->mNumMeshes; idx++) {
    aiMesh *mesh_data = scene->mMeshes[node->mMeshes[idx]];
    models_load_mesh(
      memory, model,
      array_push(&model->meshes),
      mesh_data, scene
    );
  }

  for (uint32 idx = 0; idx < node->mNumChildren; idx++) {
    models_load_model_node(memory, model, node->mChildren[idx], scene);
  }
}

void models_load_model(
  Memory *memory, Model *model,
  const char *directory, const char *filename
) {
  char path[256];
  snprintf(
    path, sizeof(path), "%s/%s", directory, filename
  );

  Assimp::Importer import;
  const aiScene *scene = import.ReadFile(
    path, aiProcess_Triangulate | aiProcess_FlipUVs
  );

  if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    log_error("ERROR::ASSIMP::%s", import.GetErrorString());
    return;
  }

  model->meshes.size = 0;
  model->meshes.max_size = 128;
  model->meshes.items = (Mesh*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(Mesh) * model->meshes.max_size,
    "meshes"
  );

  models_load_model_node(memory, model, scene->mRootNode, scene);

  memory_reset_pool(&memory->temp_memory_pool);
  memory_zero_out_pool(&memory->temp_memory_pool);
}

ModelAsset* models_make_asset_from_file(
  Memory *memory, ModelAsset *model_asset,
  const char *name, const char *directory, const char *filename
) {
  model_asset->info.name = name;
  model_asset->model.should_load_textures_from_file = true;
  model_asset->model.directory = directory;
  models_load_model(
    memory, &model_asset->model, directory, filename
  );
  return model_asset;
}

void models_add_texture_to_mesh(
  Mesh *mesh, TextureType type, uint32 texture
) {
  if (type == TEXTURE_DIFFUSE) {
    mesh->diffuse_textures[mesh->n_diffuse_textures++] = texture;
  } else if (type == TEXTURE_SPECULAR) {
    mesh->specular_textures[mesh->n_specular_textures++] = texture;
  } else if (type == TEXTURE_DEPTH) {
    mesh->depth_textures[mesh->n_depth_textures++] = texture;
  }
}

ModelAsset* models_make_asset_from_data(
  Memory *memory, ModelAsset *model_asset,
  real32 *vertex_data, uint32 n_vertices,
  uint32 *index_data, uint32 n_indices,
  const char *name,
  uint32 mode
) {
  model_asset->info.name = name;

  Model *model = &model_asset->model;

  model->should_load_textures_from_file = false;
  model->directory = "";

  model->meshes.size = 0;
  model->meshes.max_size = 1;
  model->meshes.items = (Mesh*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(Mesh) * model->meshes.max_size,
    "meshes"
  );

  Mesh *mesh = model->meshes.items;
  model->meshes.size++;

  model->meshes.items[0].mode = mode;

  // Vertices
  // NOTE: We are copying this data around for no real reason.
  // It probably doesn't matter as this is pretty much debug code,
  // but it might be good to improvei t.
  mesh->vertices.size = 0;
  mesh->vertices.max_size = n_vertices;
  mesh->vertices.items = (Vertex*)memory_push_memory_to_pool(
    &memory->temp_memory_pool, sizeof(Vertex) * mesh->vertices.max_size,
    "meshes"
  );

  for (uint32 idx = 0; idx < n_vertices; idx++) {
    Vertex *vertex = (Vertex*)array_push<Vertex>(&mesh->vertices);

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
  mesh->does_use_indices = (n_indices > 0);
  mesh->indices.size = n_indices;
  mesh->indices.max_size = n_indices;
  mesh->indices.items = index_data;

  models_setup_mesh(mesh);

  memory_reset_pool(&memory->temp_memory_pool);
  memory_zero_out_pool(&memory->temp_memory_pool);

  return model_asset;
}

void models_draw_mesh(Mesh *mesh, uint32 shader_program) {
  char uniform_name[128];
  uint32 texture_idx = 0;

  shader_set_int(shader_program, "n_diffuse_textures", mesh->n_diffuse_textures);
  shader_set_int(shader_program, "n_specular_textures", mesh->n_specular_textures);
  shader_set_int(shader_program, "n_depth_textures", mesh->n_depth_textures);

  for (uint32 idx = 0; idx < mesh->n_diffuse_textures; idx++) {
    texture_idx++;
    glActiveTexture(GL_TEXTURE0 + texture_idx);
    sprintf(uniform_name, "diffuse_textures[%d]", idx);
    shader_set_int(shader_program, uniform_name, texture_idx);
    glBindTexture(GL_TEXTURE_2D, mesh->diffuse_textures[idx]);
  }

  for (uint32 idx = 0; idx < mesh->n_specular_textures; idx++) {
    texture_idx++;
    glActiveTexture(GL_TEXTURE0 + texture_idx);
    sprintf(uniform_name, "specular_textures[%d]", idx);
    shader_set_int(shader_program, uniform_name, texture_idx);
    glBindTexture(GL_TEXTURE_2D, mesh->specular_textures[idx]);
  }

  for (uint32 idx = 0; idx < mesh->n_depth_textures; idx++) {
    texture_idx++;
    glActiveTexture(GL_TEXTURE0 + texture_idx);
    sprintf(uniform_name, "depth_textures[%d]", idx);
    shader_set_int(shader_program, uniform_name, texture_idx);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mesh->depth_textures[idx]);
  }

  glActiveTexture(GL_TEXTURE0);

  glBindVertexArray(mesh->vao);
  if (mesh->indices.size > 0) {
    glDrawElements(mesh->mode, mesh->indices.size, GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(mesh->mode, 0, mesh->vertices.size);
  }
  glBindVertexArray(0);
}

void models_draw_model(Model *model, uint32 shader_program) {
  for (uint32 idx = 0; idx < model->meshes.size; idx++) {
    models_draw_mesh(&model->meshes.items[idx], shader_program);
  }
}
