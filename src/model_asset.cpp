EntityManager *ModelAsset::entity_manager;
DrawableComponentManager *ModelAsset::drawable_component_manager;
SpatialComponentManager *ModelAsset::spatial_component_manager;
LightComponentManager *ModelAsset::light_component_manager;
BehaviorComponentManager *ModelAsset::behavior_component_manager;

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
  mesh->material = nullptr;
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
    // that, but that's fine. We don't need that much precision. Just smash the number down
    // to a uint8.
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

  if (this->model_source == ModelSource::file) {
    START_TIMER(assimp_import);
    const aiScene *scene = aiImportFile(
      this->path,
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
      memory, scene->mRootNode, scene, glm::mat4(1.0f), 0ULL
    );

    aiReleaseImport(scene);
  }

  this->is_mesh_data_loading_in_progress = false;
  this->is_mesh_data_loading_done = true;

  this->mutex.unlock();
}


void ModelAsset::copy_textures_to_pbo(PersistentPbo *persistent_pbo) {
  for (uint32 idx = 0; idx < this->materials.size; idx++) {
    Material *material = this->materials[idx];

    if (material->textures.size > 0) {
      material->copy_textures_to_pbo(persistent_pbo);
    }
  }

  this->is_texture_copying_to_pbo_done = true;
  this->is_texture_copying_to_pbo_in_progress = false;
}


void ModelAsset::bind_texture_uniforms_for_mesh(Mesh *mesh) {
  Material *material = mesh->material;
  ShaderAsset *shader_asset = material->shader_asset;

  if (material->textures.size == 0) {
    return;
  }

  if (shader_asset->type != ShaderType::depth) {
    glUseProgram(shader_asset->program);

    for (
      uint32 uniform_idx = 0;
      uniform_idx < shader_asset->n_intrinsic_uniforms;
      uniform_idx++
    ) {
      const char *uniform_name = shader_asset->intrinsic_uniform_names[uniform_idx];
      if (strcmp(uniform_name, "should_use_normal_map") == 0) {
        shader_asset->set_bool("should_use_normal_map", material->should_use_normal_map);
      } else if (strcmp(uniform_name, "albedo_static") == 0) {
        shader_asset->set_vec4("albedo_static", &material->albedo_static);
      } else if (strcmp(uniform_name, "metallic_static") == 0) {
        shader_asset->set_float("metallic_static", material->metallic_static);
      } else if (strcmp(uniform_name, "roughness_static") == 0) {
        shader_asset->set_float("roughness_static", material->roughness_static);
      } else if (strcmp(uniform_name, "ao_static") == 0) {
        shader_asset->set_float("ao_static", material->ao_static);
      }
    }

    shader_asset->reset_texture_units();

    for (uint32 idx = 0; idx < material->textures.size; idx++) {
      Texture *texture = material->textures[idx];
      const char *uniform_name = *material->texture_uniform_names[idx];
#if 0
      log_info(
        "Setting uniforms: (model %s) (uniform_name %s) "
        "(texture->texture_name %d)",
        this->name, uniform_name, texture->texture_name
      );
#endif
      shader_asset->set_int(
        uniform_name,
        shader_asset->add_texture_unit(texture->texture_name, texture->target)
      );
    }
  }

  shader_asset->did_set_texture_uniforms = true;
}


void ModelAsset::create_entities() {
  if (this->spatial_component.is_valid()) {
    this->spatial_component.entity_handle = this->entity->handle;
    this->spatial_component_manager->add(this->spatial_component);
  }

  if (this->light_component.is_valid()) {
    this->light_component.entity_handle = this->entity->handle;
    this->light_component_manager->add(this->light_component);
  }

  if (this->behavior_component.is_valid()) {
    this->behavior_component.entity_handle = this->entity->handle;
    this->behavior_component_manager->add(this->behavior_component);
  }

  if (this->meshes.size == 1) {
    this->drawable_component_manager->add(
      this->entity->handle,
      this->meshes[0],
      this->render_pass
    );
  } else if (this->meshes.size > 1) {
    for (uint32 idx = 0; idx < this->meshes.size; idx++) {
      Mesh *mesh = this->meshes[idx];

      Entity *child_entity = this->entity_manager->add(this->name);

      if (this->spatial_component.is_valid()) {
        this->spatial_component_manager->add(
          child_entity->handle,
          glm::vec3(0.0f),
          glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f)),
          glm::vec3(0.0f),
          this->entity->handle
        );
      }

      this->drawable_component_manager->add(
        child_entity->handle,
        mesh,
        this->render_pass
      );
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
    task_queue->push({TaskType::load_model, this, nullptr, memory});
  }

  // Step 2: Once the mesh data is loaded, set up vertex buffers for these meshes.
  if (this->is_mesh_data_loading_done && !this->is_vertex_buffer_setup_done) {
    for (uint32 idx = 0; idx < this->meshes.size; idx++) {
      Mesh *mesh = this->meshes[idx];
      setup_mesh_vertex_buffers_for_file_source(mesh, &mesh->vertices, &mesh->indices);
    }
    this->is_vertex_buffer_setup_done = true;
  }

  if (this->is_mesh_data_loading_done && this->materials.size > 0) {
    // Step 3: Once the mesh data is loaded, bind materials for their respective meshes.
    if (this->is_mesh_data_loading_done && !this->is_shader_setting_done) {
      for (uint32 idx_material = 0; idx_material < this->materials.size; idx_material++) {
        Material *material = this->materials[idx_material];
        for (uint32 idx_mesh = 0; idx_mesh < this->meshes.size; idx_mesh++) {
          Mesh *mesh = this->meshes[idx_mesh];
          uint8 mesh_number = pack_get(&mesh->indices_pack, 0);
          // For our model's mesh number `mesh_number`, we want to choose
          // material `idx_mesh` such that `mesh_number == idx_mesh`, i.e.
          // we choose the 4th material for mesh number 4.
          // However, if we have more meshes than materials, the extra
          // meshes all get material number 0.
          if (
            mesh_number == idx_material ||
            mesh_number >= this->materials.size && idx_material == 0
          ) {
            mesh->material = material;
          }
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
      for (uint32 idx = 0; idx < this->materials.size; idx++) {
        Material *material = this->materials[idx];
        if (material->textures.size > 0) {
          for (
            uint32 texture_idx = 0;
            texture_idx < material->textures.size;
            texture_idx++
          ) {
            Texture *texture = material->textures[texture_idx];
            if (!texture->texture_name) {
              should_try_to_copy_textures = true;
              break;
            }
          }
        }
      }

      if (should_try_to_copy_textures) {
        this->is_texture_copying_to_pbo_in_progress = true;
        task_queue->push({TaskType::copy_textures_to_pbo, this, persistent_pbo, nullptr});
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

      for (uint32 idx = 0; idx < this->materials.size; idx++) {
        Material *material = this->materials[idx];

        if (
          material->textures.size > 0 &&
          !material->have_textures_been_generated
        ) {
          material->generate_textures_from_pbo(
            persistent_pbo, texture_name_pool
          );
        }
      }

      this->is_texture_creation_done = true;
      END_TIMER_MIN(copy_pbo_to_texture, 5);
    }

    // Step 6: Bind the texture uniforms.
    // NOTE: Because the shader might be reloaded at any time, we need to
    // check whether or not we need to set any uniforms every time.
    if (this->is_texture_creation_done) {
      for (uint32 idx = 0; idx < this->materials.size; idx++) {
        Material *material = this->materials[idx];

        if (
          material->textures.size > 0 &&
          !material->shader_asset->did_set_texture_uniforms
        ) {
          for (uint32 idx_mesh = 0; idx_mesh < this->meshes.size; idx_mesh++) {
            Mesh *mesh = this->meshes[idx_mesh];
            if (!mesh->material->shader_asset->did_set_texture_uniforms) {
              bind_texture_uniforms_for_mesh(mesh);
            }
          }
        }
      }
    }

    // Step 7: Create the entities.
    if (
      this->is_texture_creation_done &&
      !this->is_entity_creation_done
    ) {
      create_entities();
      this->is_entity_creation_done = true;
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


ModelAsset::ModelAsset(
  Memory *memory,
  ModelSource model_source,
  const char *name,
  const char *path,
  RenderPass::Flag render_pass,
  Entity *entity
) :
  name(name),
  model_source(model_source),
  path(path),
  meshes(&memory->asset_memory_pool, MAX_N_MESHES, "meshes"),
  materials(&memory->asset_memory_pool, MAX_N_MATERIALS, "materials"),
  render_pass(render_pass),
  entity(entity)
{
  // NOTE: We do not load ModelSource::file models here.
  // They will be loaded gradually in `::prepare_for_draw()`.
}


ModelAsset::ModelAsset(
  Memory *memory,
  ModelSource model_source,
  real32 *vertex_data, uint32 n_vertices,
  uint32 *index_data, uint32 n_indices,
  const char *name,
  GLenum mode,
  RenderPass::Flag render_pass,
  Entity *entity
) :
  name(name),
  model_source(model_source),
  path(""),
  meshes(&memory->asset_memory_pool, MAX_N_MESHES, "meshes"),
  materials(&memory->asset_memory_pool, MAX_N_MATERIALS, "materials"),
  render_pass(render_pass),
  entity(entity)
{
  Mesh *mesh = this->meshes.push();
  mesh->transform = glm::mat4(1.0f);
  mesh->material = nullptr;
  mesh->mode = mode;
  mesh->n_vertices = n_vertices;
  mesh->n_indices = n_indices;
  mesh->indices_pack = 0UL;

  setup_mesh_vertex_buffers_for_data_source(
    mesh, vertex_data, n_vertices, index_data, n_indices
  );
  this->is_mesh_data_loading_done = true;
  this->is_vertex_buffer_setup_done = true;
}
