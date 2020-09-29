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


void models_setup_mesh_vertex_buffers(
  Mesh *mesh, Array<Vertex> *vertices, Array<uint32> *indices
) {
  glGenVertexArrays(1, &mesh->vao);
  glGenBuffers(1, &mesh->vbo);
  glGenBuffers(1, &mesh->ebo);

  glBindVertexArray(mesh->vao);

  glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
  glBufferData(
    GL_ARRAY_BUFFER, sizeof(Vertex) * vertices->size,
    vertices->items, GL_STATIC_DRAW
  );

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32) * indices->size,
    indices->items, GL_STATIC_DRAW
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
  Mesh *mesh, aiMesh *mesh_data, const aiScene *scene,
  Array<Vertex> *vertices
) {
  // NOTE: This is probably #slow. We're copying data around from one format
  // to another for no real reason. There would probably be a smart way to
  // use the original data, but it's not a big bottleneck according to the
  // profiler.
  array_init<Vertex>(&memory->temp_memory_pool, vertices, mesh_data->mNumVertices);

  if (!mesh_data->mNormals) {
    log_warning("Model does not have normals.");
  }

  mesh->n_vertices = mesh_data->mNumVertices;

  for (uint32 idx = 0; idx < mesh_data->mNumVertices; idx++) {
    Vertex *vertex = (Vertex*)array_push<Vertex>(vertices);

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
  Mesh *mesh, aiMesh *mesh_data, const aiScene *scene,
  Array<uint32> *indices
) {
  uint32 n_indices = 0;
  for (uint32 idx_face = 0; idx_face < mesh_data->mNumFaces; idx_face++) {
    aiFace face = mesh_data->mFaces[idx_face];
    n_indices += face.mNumIndices;
  }

  mesh->n_indices = n_indices;
  array_init<uint32>(&memory->temp_memory_pool, indices, n_indices);

  for (uint32 idx_face = 0; idx_face < mesh_data->mNumFaces; idx_face++) {
    aiFace face = mesh_data->mFaces[idx_face];
    for (uint32 idx_index = 0; idx_index < face.mNumIndices; idx_index++) {
      array_push(indices, face.mIndices[idx_index]);
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
  Array<Vertex> vertices;
  Array<uint32> indices;
  models_init_mesh(mesh, GL_TRIANGLES);
  mesh->transform = transform;
  mesh->indices_pack = indices_pack;
  models_load_mesh_vertices(
    memory, model, mesh, mesh_data, scene, &vertices
  );
  models_load_mesh_indices(
    memory, model, mesh, mesh_data, scene, &indices
  );
#if 0
  if (model->should_load_textures_from_file) {
    models_load_mesh_textures(
      memory, model, mesh, mesh_data, scene
    );
  }
#endif
  models_setup_mesh_vertex_buffers(mesh, &vertices, &indices);
}


void models_load_model_node(
  Memory *memory, Model *model,
  aiNode *node, const aiScene *scene,
  glm::mat4 accumulated_transform,
  Pack indices_pack
) {
  glm::mat4 node_transform = Util::aimatrix4x4_to_glm(&node->mTransformation);
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
  strcpy(path, directory);
  strcat(path, "/");
  strcat(path, filename);

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

  // NOTE: We are copying this data around for no real reason.
  // It probably doesn't matter as this is pretty much debug code,
  // but it might be good to improve it.

  // Vertices
  Array<Vertex> vertices;
  array_init<Vertex>(&memory->temp_memory_pool, &vertices, n_vertices);
  mesh->n_vertices = n_vertices;

  for (uint32 idx = 0; idx < n_vertices; idx++) {
    Vertex *vertex = (Vertex*)array_push<Vertex>(&vertices);

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
  Array<uint32> indices;
  array_init<uint32>(&memory->temp_memory_pool, &indices, n_indices);
  indices.size = n_indices;
  indices.items = index_data;
  mesh->n_indices = n_indices;

  models_setup_mesh_vertex_buffers(mesh, &vertices, &indices);

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
  uint32 texture_set_id = (uint32)Util::random(0, UINT32_MAX);

  TextureSet *texture_set = array_push(&model->texture_sets);

  models_init_texture_set(texture_set, texture_set_id);
  return texture_set;
}


void models_bind_texture_uniforms_for_mesh(Mesh *mesh) {
  TextureSet *texture_set = mesh->texture_set;
  ShaderAsset *shader_asset = mesh->shader_asset;
  Shader *shader = &shader_asset->shader;

  if (shader->did_set_texture_uniforms || !texture_set) {
    return;
  }

  if (shader->type == SHADER_ENTITY) {
    glUseProgram(shader->program);

    bool should_use_normal_map = texture_set->normal_texture != 0;
    shader_set_bool(shader, "should_use_normal_map", should_use_normal_map);

    shader_set_vec4(shader, "albedo_static", &texture_set->albedo_static);
    shader_set_float(shader, "metallic_static", texture_set->metallic_static);
    shader_set_float(shader, "roughness_static", texture_set->roughness_static);
    shader_set_float(shader, "ao_static", texture_set->ao_static);

    shader_set_int(
      shader, "albedo_texture",
      shader_add_texture_unit(shader, texture_set->albedo_texture, GL_TEXTURE_2D)
    );

    shader_set_int(
      shader, "metallic_texture",
      shader_add_texture_unit(shader, texture_set->metallic_texture, GL_TEXTURE_2D)
    );

    shader_set_int(
      shader, "roughness_texture",
      shader_add_texture_unit(shader, texture_set->roughness_texture, GL_TEXTURE_2D)
    );

    shader_set_int(
      shader, "ao_texture",
      shader_add_texture_unit(shader, texture_set->ao_texture, GL_TEXTURE_2D)
    );

    shader_set_int(
      shader, "normal_texture",
      shader_add_texture_unit(shader, texture_set->normal_texture, GL_TEXTURE_2D)
    );
  } else if (shader->type == SHADER_LIGHTING) {
    glUseProgram(shader->program);

    shader_set_int(shader, "n_depth_textures", texture_set->n_depth_textures);

    for (uint32 idx = 0; idx < MAX_N_SHADOW_FRAMEBUFFERS; idx++) {
      shader_set_int(
        shader, DEPTH_TEXTURE_UNIFORM_NAMES[idx],
        shader_add_texture_unit(
          shader, texture_set->depth_textures[idx], GL_TEXTURE_CUBE_MAP
        )
      );
    }

    shader_set_int(
      shader, "g_position_texture",
      shader_add_texture_unit(shader, texture_set->g_position_texture, GL_TEXTURE_2D)
    );

    shader_set_int(
      shader, "g_normal_texture",
      shader_add_texture_unit(shader, texture_set->g_normal_texture, GL_TEXTURE_2D)
    );

    shader_set_int(
      shader, "g_albedo_texture",
      shader_add_texture_unit(shader, texture_set->g_albedo_texture, GL_TEXTURE_2D)
    );

    shader_set_int(
      shader, "g_pbr_texture",
      shader_add_texture_unit(shader, texture_set->g_pbr_texture, GL_TEXTURE_2D)
    );
  } else {
    log_info(
      "Tried to set texture uniforms, but there is nothing to do for this shader: \"%s\"",
      shader_asset->info.name
    );
  }

  shader->did_set_texture_uniforms = true;
}


void models_set_shader_asset_for_mesh(Model *model, uint32 idx_mesh, ShaderAsset *shader_asset) {
  Mesh *mesh = &model->meshes.items[idx_mesh];
  mesh->shader_asset = shader_asset;
  models_bind_texture_uniforms_for_mesh(mesh);
}


void models_set_shader_asset(Model *model, ShaderAsset *shader_asset) {
  for (uint32 idx_mesh = 0; idx_mesh < model->meshes.size; idx_mesh++) {
    models_set_shader_asset_for_mesh(model, idx_mesh, shader_asset);
  }
}


void models_set_shader_asset_for_node_idx(
  Model *model, ShaderAsset *shader_asset, uint8 node_depth, uint8 node_idx
) {
  for (uint32 idx_mesh = 0; idx_mesh < model->meshes.size; idx_mesh++) {
    Mesh *mesh = &model->meshes.items[idx_mesh];
    if (pack_get(&mesh->indices_pack, node_depth) == node_idx) {
      models_set_shader_asset_for_mesh(model, idx_mesh, shader_asset);
    }
  }
}


void models_set_texture_set(Model *model, TextureSet *texture_set) {
  for (uint32 idx_mesh = 0; idx_mesh < model->meshes.size; idx_mesh++) {
    Mesh *mesh = &model->meshes.items[idx_mesh];
    mesh->texture_set = texture_set;
  }
}


void models_set_texture_set_for_node_idx(
  Model *model, TextureSet *texture_set, uint8 node_depth, uint8 node_idx
) {
  for (uint32 idx_mesh = 0; idx_mesh < model->meshes.size; idx_mesh++) {
    Mesh *mesh = &model->meshes.items[idx_mesh];
    if (pack_get(&mesh->indices_pack, node_depth) == node_idx) {
      mesh->texture_set = texture_set;
    }
  }
}


void models_draw_mesh(
  Mesh *mesh, glm::mat4 *model_matrix,
  uint32 *last_used_texture_set_id, uint32 *last_used_shader_program
) {
}


void models_draw_model(
  ModelAsset *model_asset, glm::mat4 *model_matrix
) {
  Model *model = &model_asset->model;
  uint32 last_used_texture_set_id = 0;
  uint32 last_used_shader_program = 0;

  for (uint32 idx = 0; idx < model->meshes.size; idx++) {
    Mesh *mesh = &model->meshes.items[idx];
    Shader *shader = &mesh->shader_asset->shader;

    if (shader->program != last_used_shader_program) {
      glUseProgram(shader->program);
      last_used_shader_program = shader->program;

      if (shader->type == SHADER_ENTITY || shader->type == SHADER_OTHER_OBJECT) {
        shader_set_mat4(shader, "model", model_matrix);
      }
    }

    if (mesh->texture_set && (mesh->texture_set->id != last_used_texture_set_id)) {
      for (
        uint32 texture_idx = 1; texture_idx < shader->n_texture_units + 1; texture_idx++
      ) {
        if (shader->texture_units[texture_idx] != 0) {
          glActiveTexture(GL_TEXTURE0 + texture_idx);
          glBindTexture(shader->texture_unit_types[texture_idx],
            shader->texture_units[texture_idx]);
        }
      }
      last_used_texture_set_id = mesh->texture_set->id;
    }

    if (shader->type == SHADER_ENTITY) {
      shader_set_mat4(shader, "mesh_transform", &mesh->transform);
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


void models_draw_model_in_depth_mode(
  ModelAsset *model_asset, glm::mat4 *model_matrix,
  ShaderAsset *entity_depth_shader_asset, uint32 shadow_light_idx
) {
  Model *model = &model_asset->model;

  for (uint32 idx = 0; idx < model->meshes.size; idx++) {
    Mesh *mesh = &model->meshes.items[idx];
    Shader *shader = &entity_depth_shader_asset->shader;

    glUseProgram(shader->program);
    shader_set_mat4(shader, "model", model_matrix);
    shader_set_mat4(shader, "mesh_transform", &mesh->transform);
    shader_set_int(shader, "shadow_light_idx", shadow_light_idx);

    glBindVertexArray(mesh->vao);
    if (mesh->n_indices > 0) {
      glDrawElements(mesh->mode, mesh->n_indices, GL_UNSIGNED_INT, 0);
    } else {
      glDrawArrays(mesh->mode, 0, mesh->n_vertices);
    }
    glBindVertexArray(0);
  }
}
