uint32 models_load_texture_from_file(const char *path, bool should_flip) {
  uint32 texture_id;
  glGenTextures(1, &texture_id);

  int32 width, height, n_components;
  unsigned char *data = util_load_image(
    path, &width, &height, &n_components, should_flip
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

uint32 models_load_texture_from_file(const char *path) {
  return models_load_texture_from_file(path, true);
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
      tex_coords.y = 1 - mesh_data->mTextureCoords[0][idx].y;
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

#if 0
void models_load_mesh_textures(
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
    util_join(texture_path, model->directory, "/", texture_filename_aistring.C_Str());
    mesh->diffuse_textures[idx] = models_load_texture_from_file(
      texture_path
    );
  }

  for (uint32 idx = 0; idx < mesh->n_specular_textures; idx++) {
    material->GetTexture(aiTextureType_SPECULAR, idx, &texture_filename_aistring);
    util_join(texture_path, model->directory, "/", texture_filename_aistring.C_Str());
    mesh->specular_textures[idx] = models_load_texture_from_file(
      texture_path
    );
  }
}
#endif

void models_init_mesh(Mesh *mesh, uint32 mode) {
  mesh->mode = mode;
  mesh->is_screenquad = false;

  mesh->transform = glm::mat4(1.0f);

  mesh->albedo_texture = 0;
  mesh->metallic_texture = 0;
  mesh->roughness_texture = 0;
  mesh->ao_texture = 0;
  mesh->normal_texture = 0;

  mesh->albedo_static = glm::vec4(-1.0f, -1.0f, -1.0f, -1.0f);
  mesh->metallic_static = -1.0f;
  mesh->roughness_static = -1.0f;
  mesh->ao_static = -1.0f;
}

void models_load_mesh(
  Memory *memory, Model *model,
  Mesh *mesh, aiMesh *mesh_data, const aiScene *scene,
  glm::mat4 transform,
  Pack indices_pack
) {
  models_init_mesh(mesh, GL_TRIANGLES);
  mesh->transform = transform;
  mesh->indices_pack = indices_pack;
  models_load_mesh_vertices(
    memory, model, mesh, mesh_data, scene
  );
  models_load_mesh_indices(
    memory, model, mesh, mesh_data, scene
  );
#if 0
  if (model->should_load_textures_from_file) {
    models_load_mesh_textures(
      memory, model, mesh, mesh_data, scene
    );
  }
#endif
  models_setup_mesh(mesh);
}

void models_load_model_node(
  Memory *memory, Model *model,
  aiNode *node, const aiScene *scene,
  glm::mat4 accumulated_transform,
  Pack indices_pack
) {
  glm::mat4 node_transform = aimatrix4x4_to_glm(&node->mTransformation);
  glm::mat4 transform = accumulated_transform * node_transform;

  for (uint32 idx = 0; idx < node->mNumMeshes; idx++) {
    aiMesh *mesh_data = scene->mMeshes[node->mMeshes[idx]];
    models_load_mesh(
      memory, model,
      array_push(&model->meshes),
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
    models_load_model_node(
      memory, model, node->mChildren[idx], scene, transform, new_indices_pack
    );
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

  const aiScene *scene = aiImportFile(
    path,
    aiProcess_Triangulate |
    aiProcess_FlipUVs
#if 0
    aiProcess_JoinIdenticalVertices |
    aiProcess_SortByPType |
    aiProcess_GenNormals |
    // NOTE: Uncomment this when changing to proper normal mapping.
    aiProcess_CalcTangentSpace |
#endif
  );

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    log_error("assimp error: %s", aiGetErrorString());
    return;
  }

  model->meshes.size = 0;
  model->meshes.max_size = MAX_N_MESHES;
  model->meshes.items = (Mesh*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(Mesh) * model->meshes.max_size,
    "meshes"
  );

  models_load_model_node(
    memory, model, scene->mRootNode, scene, glm::mat4(1.0f), 0ULL
  );

  aiReleaseImport(scene);
}

ModelAsset* models_make_asset_from_file(
  Memory *memory, ModelAsset *model_asset,
  const char *name, const char *directory, const char *filename
) {
  model_asset->info.name = name;
#if 0
  model_asset->model.should_load_textures_from_file = true;
#endif
  model_asset->model.directory = directory;
  models_load_model(
    memory, &model_asset->model, directory, filename
  );
  memory_reset_pool(&memory->temp_memory_pool);
  return model_asset;
}

void models_add_texture(
  Model *model, uint32 idx_mesh, TextureType type, uint32 texture
) {
  Mesh *mesh = &model->meshes.items[idx_mesh];
  if (type == TEXTURE_DIFFUSE || type == TEXTURE_SPECULAR) {
    log_warning("No diffuse or specular textures here, buddy!");
  } else if (type == TEXTURE_DEPTH) {
    mesh->depth_textures[mesh->n_depth_textures++] = texture;
  } else if (type == TEXTURE_ALBEDO) {
    mesh->albedo_texture = texture;
  } else if (type == TEXTURE_METALLIC) {
    mesh->metallic_texture = texture;
  } else if (type == TEXTURE_ROUGHNESS) {
    mesh->roughness_texture = texture;
  } else if (type == TEXTURE_AO) {
    mesh->ao_texture = texture;
  } else if (type == TEXTURE_NORMAL) {
    mesh->normal_texture = texture;
  } else if (type == TEXTURE_G_POSITION) {
    mesh->g_position_texture = texture;
  } else if (type == TEXTURE_G_NORMAL) {
    mesh->g_normal_texture = texture;
  } else if (type == TEXTURE_G_ALBEDO) {
    mesh->g_albedo_texture = texture;
  } else if (type == TEXTURE_G_PBR) {
    mesh->g_pbr_texture = texture;
  } else {
    log_warning("Can't bind that texture type, pal.");
  }
}

void models_add_texture(
  Model *model, TextureType type, uint32 texture
) {
  for (uint32 idx_mesh = 0; idx_mesh < model->meshes.size; idx_mesh++) {
    models_add_texture(model, idx_mesh, type, texture);
  }
}

void models_add_texture_for_node_idx(
  Model *model, TextureType type, uint32 texture, uint8 node_depth, uint8 node_idx
) {
  for (uint32 idx_mesh = 0; idx_mesh < model->meshes.size; idx_mesh++) {
    Mesh *mesh = &model->meshes.items[idx_mesh];
    if (pack_get(&mesh->indices_pack, node_depth) == node_idx) {
      models_add_texture(model, idx_mesh, type, texture);
    }
  }
}

void models_set_static_pbr(
  Model *model, uint32 idx_mesh,
  glm::vec4 albedo, real32 metallic, real32 roughness, real32 ao
) {
  Mesh *mesh = &model->meshes.items[idx_mesh];
  mesh->albedo_static = albedo;
  mesh->metallic_static = metallic;
  mesh->roughness_static = roughness;
  mesh->ao_static = ao;
}

void models_set_static_pbr(
  Model *model,
  glm::vec4 albedo, real32 metallic, real32 roughness, real32 ao
) {
  for (uint32 idx_mesh = 0; idx_mesh < model->meshes.size; idx_mesh++) {
    models_set_static_pbr(model, idx_mesh, albedo, metallic, roughness, ao);
  }
}

void models_set_is_screenquad(Model *model) {
  for (uint32 idx = 0; idx < model->meshes.size; idx++) {
    Mesh *mesh = &model->meshes.items[idx];
    mesh->is_screenquad = true;
  }
}

ModelAsset* models_make_asset_from_data(
  Memory *memory, ModelAsset *model_asset,
  real32 *vertex_data, uint32 n_vertices,
  uint32 *index_data, uint32 n_indices,
  const char *name,
  GLenum mode
) {
  model_asset->info.name = name;

  Model *model = &model_asset->model;

#if 0
  model->should_load_textures_from_file = false;
#endif
  model->directory = "";

  model->meshes.size = 0;
  model->meshes.max_size = 1;
  model->meshes.items = (Mesh*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(Mesh) * model->meshes.max_size,
    "meshes"
  );

  Mesh *mesh = model->meshes.items;
  model->meshes.size++;

  models_init_mesh(&model->meshes.items[0], mode);

  // Vertices
  // NOTE: We are copying this data around for no real reason.
  // It probably doesn't matter as this is pretty much debug code,
  // but it might be good to improve it.
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
  mesh->indices.size = n_indices;
  mesh->indices.max_size = n_indices;
  mesh->indices.items = index_data;

  models_setup_mesh(mesh);

  memory_reset_pool(&memory->temp_memory_pool);

  return model_asset;
}

void model_set_uniforms_for_mesh(Mesh *mesh, Shader *shader) {
  uint32 texture_idx = 0;

  shader_set_mat4(shader, UNIFORM_MESH_TRANSFORM, &mesh->transform);
  shader_set_int(shader, UNIFORM_N_DEPTH_TEXTURES, mesh->n_depth_textures);

  for (uint32 idx = 0; idx < MAX_N_SHADOW_FRAMEBUFFERS; idx++) {
    glActiveTexture(GL_TEXTURE0 + (++texture_idx));
    shader_set_int(shader, DEPTH_TEXTURE_UNIFORM_NAMES[idx], texture_idx);
    glBindTexture(GL_TEXTURE_CUBE_MAP, mesh->depth_textures[idx]);
  }

  if (mesh->is_screenquad) {
    glActiveTexture(GL_TEXTURE0 + (++texture_idx));
    shader_set_int(shader, UNIFORM_G_POSITION_TEXTURE, texture_idx);
    glBindTexture(GL_TEXTURE_2D, mesh->g_position_texture);

    glActiveTexture(GL_TEXTURE0 + (++texture_idx));
    shader_set_int(shader, UNIFORM_G_NORMAL_TEXTURE, texture_idx);
    glBindTexture(GL_TEXTURE_2D, mesh->g_normal_texture);

    glActiveTexture(GL_TEXTURE0 + (++texture_idx));
    shader_set_int(shader, UNIFORM_G_ALBEDO_TEXTURE, texture_idx);
    glBindTexture(GL_TEXTURE_2D, mesh->g_albedo_texture);

    glActiveTexture(GL_TEXTURE0 + (++texture_idx));
    shader_set_int(shader, UNIFORM_G_PBR_TEXTURE, texture_idx);
    glBindTexture(GL_TEXTURE_2D, mesh->g_pbr_texture);
  } else {
    shader_set_vec4(shader, UNIFORM_ALBEDO_STATIC, &mesh->albedo_static);
    shader_set_float(shader, UNIFORM_METALLIC_STATIC, mesh->metallic_static);
    shader_set_float(shader, UNIFORM_ROUGHNESS_STATIC, mesh->roughness_static);
    shader_set_float(shader, UNIFORM_AO_STATIC, mesh->ao_static);

    glActiveTexture(GL_TEXTURE0 + (++texture_idx));
    shader_set_int(shader, UNIFORM_ALBEDO_TEXTURE, texture_idx);
    glBindTexture(GL_TEXTURE_2D, mesh->albedo_texture);

    glActiveTexture(GL_TEXTURE0 + (++texture_idx));
    shader_set_int(shader, UNIFORM_METALLIC_TEXTURE, texture_idx);
    glBindTexture(GL_TEXTURE_2D, mesh->metallic_texture);

    glActiveTexture(GL_TEXTURE0 + (++texture_idx));
    shader_set_int(shader, UNIFORM_ROUGHNESS_TEXTURE, texture_idx);
    glBindTexture(GL_TEXTURE_2D, mesh->roughness_texture);

    glActiveTexture(GL_TEXTURE0 + (++texture_idx));
    shader_set_int(shader, UNIFORM_AO_TEXTURE, texture_idx);
    glBindTexture(GL_TEXTURE_2D, mesh->ao_texture);

    glActiveTexture(GL_TEXTURE0 + (++texture_idx));
    shader_set_int(shader, UNIFORM_NORMAL_TEXTURE, texture_idx);
    glBindTexture(GL_TEXTURE_2D, mesh->normal_texture);
  }

  if (mesh->normal_texture != 0) {
    shader_set_bool(shader, UNIFORM_SHOULD_USE_NORMAL_MAP, true);
  } else {
    shader_set_bool(shader, UNIFORM_SHOULD_USE_NORMAL_MAP, false);
  }
}

void models_draw_mesh(Mesh *mesh, Shader *shader) {
  model_set_uniforms_for_mesh(mesh, shader);

  glBindVertexArray(mesh->vao);
  if (mesh->indices.size > 0) {
    glDrawElements(mesh->mode, mesh->indices.size, GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(mesh->mode, 0, mesh->vertices.size);
  }
  glBindVertexArray(0);
}

void models_draw_model(Model *model, Shader *shader) {
  for (uint32 idx = 0; idx < model->meshes.size; idx++) {
    models_draw_mesh(&model->meshes.items[idx], shader);
  }
}
