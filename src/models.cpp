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


void models_init(Model *model, Memory *memory, const char *directory) {
  model->should_load_textures_from_file = false;
  model->directory = directory;

  model->meshes.size = 0;
  model->meshes.max_size = MAX_N_MESHES;
  model->meshes.items = (Mesh*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(Mesh) * model->meshes.max_size,
    "meshes"
  );

  model->texture_sets.size = 0;
  model->texture_sets.max_size = MAX_N_TEXTURE_SETS;
  model->texture_sets.items = (TextureSet*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(TextureSet) * model->texture_sets.max_size,
    "texture_sets"
  );
}


void models_setup_mesh_vertex_buffers(Mesh *mesh) {
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


void models_load_mesh_vertices(
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


void models_load_mesh_indices(
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
  mesh->transform = glm::mat4(1.0f);
  mesh->texture_set = nullptr;
  mesh->mode = mode;
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
  models_setup_mesh_vertex_buffers(mesh);
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
  models_init(&model_asset->model, memory, directory);
  models_load_model(
    memory, &model_asset->model, directory, filename
  );
  memory_reset_pool(&memory->temp_memory_pool);
  return model_asset;
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
  models_init(model, memory, "");

  Mesh *mesh = array_push<Mesh>(&model->meshes);
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

  models_setup_mesh_vertex_buffers(mesh);

  memory_reset_pool(&memory->temp_memory_pool);

  return model_asset;
}


void models_init_texture_set(TextureSet *texture_set, uint32 id) {
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


TextureSet* models_add_texture_set(Model *model) {
  // NOTE: As an ID for the texture set, we take its index in the
  // texture set array, plus one. This means that valid indices start from
  // 1 on. This is so we can use 0 as an "unset" value. We might want to do
  // something else in the future, but this should do.
  /* uint32 texture_set_id = model->texture_sets.size + 1; */

  // We initialise texture_set ids to something random and hopefully
  // more or less globally unique.
  // TODO: Change this to something actually less likely to collide.
  uint32 texture_set_id = (uint32)util_random(0, UINT32_MAX);

  TextureSet *texture_set = array_push(&model->texture_sets);

  models_init_texture_set(texture_set, texture_set_id);
  return texture_set;
}


void models_set_texture_set(Model *model, uint32 idx_mesh, TextureSet *texture_set) {
  Mesh *mesh = &model->meshes.items[idx_mesh];
  mesh->texture_set = texture_set;
}


void models_set_texture_set(Model *model, TextureSet *texture_set) {
  for (uint32 idx_mesh = 0; idx_mesh < model->meshes.size; idx_mesh++) {
    models_set_texture_set(model, idx_mesh, texture_set);
  }
}


void models_set_texture_set_for_node_idx(
  Model *model, TextureSet *texture_set, uint8 node_depth, uint8 node_idx
) {
  for (uint32 idx_mesh = 0; idx_mesh < model->meshes.size; idx_mesh++) {
    Mesh *mesh = &model->meshes.items[idx_mesh];
    if (pack_get(&mesh->indices_pack, node_depth) == node_idx) {
      models_set_texture_set(model, idx_mesh, texture_set);
    }
  }
}


#if 0
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
#endif


void models_set_uniforms_for_mesh(
  Mesh *mesh, ShaderAsset *shader_asset, RenderMode render_mode
) {
  Shader *shader = &shader_asset->shader;
  uint32 texture_idx = 0;

  bool is_lighting = (strcmp(shader_asset->info.name, "lighting") == 0);
  bool is_entity = (strcmp(shader_asset->info.name, "entity") == 0);
  bool is_entity_depth = (strcmp(shader_asset->info.name, "entity_depth") == 0);

  if (is_entity || is_entity_depth) {
    shader_set_mat4(shader, "mesh_transform", &mesh->transform);
  }

  if (mesh->texture_set && mesh->texture_set->id != shader->last_bound_texture_set_id) {
    shader->last_bound_texture_set_id = mesh->texture_set->id;

    if (is_entity) {
      shader_set_vec4(shader, "albedo_static", &mesh->texture_set->albedo_static);
      shader_set_float(shader, "metallic_static", mesh->texture_set->metallic_static);
      shader_set_float(shader, "roughness_static", mesh->texture_set->roughness_static);
      shader_set_float(shader, "ao_static", mesh->texture_set->ao_static);

      glActiveTexture(GL_TEXTURE0 + (++texture_idx));
      shader_set_int(shader, "albedo_texture", texture_idx);
      glBindTexture(GL_TEXTURE_2D, mesh->texture_set->albedo_texture);

      glActiveTexture(GL_TEXTURE0 + (++texture_idx));
      shader_set_int(shader, "metallic_texture", texture_idx);
      glBindTexture(GL_TEXTURE_2D, mesh->texture_set->metallic_texture);

      glActiveTexture(GL_TEXTURE0 + (++texture_idx));
      shader_set_int(shader, "roughness_texture", texture_idx);
      glBindTexture(GL_TEXTURE_2D, mesh->texture_set->roughness_texture);

      glActiveTexture(GL_TEXTURE0 + (++texture_idx));
      shader_set_int(shader, "ao_texture", texture_idx);
      glBindTexture(GL_TEXTURE_2D, mesh->texture_set->ao_texture);

      glActiveTexture(GL_TEXTURE0 + (++texture_idx));
      shader_set_int(shader, "normal_texture", texture_idx);
      glBindTexture(GL_TEXTURE_2D, mesh->texture_set->normal_texture);

      if (mesh->texture_set->normal_texture != 0) {
        shader_set_bool(shader, "should_use_normal_map", true);
      } else {
        shader_set_bool(shader, "should_use_normal_map", false);
      }
    } else if (is_lighting) {
      shader_set_int(shader, "n_depth_textures", mesh->texture_set->n_depth_textures);

      for (uint32 idx = 0; idx < MAX_N_SHADOW_FRAMEBUFFERS; idx++) {
      /* for (uint32 idx = 0; idx < mesh->texture_set->n_depth_textures; idx++) { */
        log_info(
          "setting %s to %d",
          DEPTH_TEXTURE_UNIFORM_NAMES[idx],
          mesh->texture_set->depth_textures[idx]
        );
        glActiveTexture(GL_TEXTURE0 + (++texture_idx));
        shader_set_int(shader, DEPTH_TEXTURE_UNIFORM_NAMES[idx], texture_idx);
        glBindTexture(GL_TEXTURE_CUBE_MAP, mesh->texture_set->depth_textures[idx]);
      }

      glActiveTexture(GL_TEXTURE0 + (++texture_idx));
      shader_set_int(shader, "g_position_texture", texture_idx);
      glBindTexture(GL_TEXTURE_2D, mesh->texture_set->g_position_texture);

      glActiveTexture(GL_TEXTURE0 + (++texture_idx));
      shader_set_int(shader, "g_normal_texture", texture_idx);
      glBindTexture(GL_TEXTURE_2D, mesh->texture_set->g_normal_texture);

      glActiveTexture(GL_TEXTURE0 + (++texture_idx));
      shader_set_int(shader, "g_albedo_texture", texture_idx);
      glBindTexture(GL_TEXTURE_2D, mesh->texture_set->g_albedo_texture);

      glActiveTexture(GL_TEXTURE0 + (++texture_idx));
      shader_set_int(shader, "g_pbr_texture", texture_idx);
      glBindTexture(GL_TEXTURE_2D, mesh->texture_set->g_pbr_texture);
    }
  }
}


void models_draw_mesh(
  Mesh *mesh, ShaderAsset *shader_asset, RenderMode render_mode
) {
  models_set_uniforms_for_mesh(mesh, shader_asset, render_mode);

  glBindVertexArray(mesh->vao);
  if (mesh->indices.size > 0) {
    glDrawElements(mesh->mode, mesh->indices.size, GL_UNSIGNED_INT, 0);
  } else {
    glDrawArrays(mesh->mode, 0, mesh->vertices.size);
  }
  glBindVertexArray(0);
}


void models_draw_model(Model *model, ShaderAsset *shader_asset, RenderMode render_mode) {
  for (uint32 idx = 0; idx < model->meshes.size; idx++) {
    models_draw_mesh(
      &model->meshes.items[idx], shader_asset, render_mode
    );
  }
}
