void Models::setup_mesh_vertex_buffers_for_data_source(
  Models::Mesh *mesh,
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


void Models::setup_mesh_vertex_buffers_for_file_source(
  Models::Mesh *mesh, Array<Vertex> *vertices, Array<uint32> *indices
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

  Memory::destroy_memory_pool(&mesh->temp_memory_pool);
}


void Models::load_mesh(
  Models::Mesh *mesh, aiMesh *mesh_data, const aiScene *scene,
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
  mesh->vertices = Array<Vertex>(
    &mesh->temp_memory_pool, mesh_data->mNumVertices, "mesh_vertices"
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
    &mesh->temp_memory_pool, n_indices, "mesh_indices"
  );

  for (uint32 idx_face = 0; idx_face < mesh_data->mNumFaces; idx_face++) {
    aiFace face = mesh_data->mFaces[idx_face];
    for (uint32 idx_index = 0; idx_index < face.mNumIndices; idx_index++) {
      mesh->indices.push(face.mIndices[idx_index]);
    }
  }
}


void Models::load_node(
  Models::ModelAsset *model_asset,
  aiNode *node, const aiScene *scene,
  glm::mat4 accumulated_transform, Pack indices_pack
) {
  glm::mat4 node_transform = Util::aimatrix4x4_to_glm(&node->mTransformation);
  glm::mat4 transform = accumulated_transform * node_transform;

  for (uint32 idx = 0; idx < node->mNumMeshes; idx++) {
    aiMesh *mesh_data = scene->mMeshes[node->mMeshes[idx]];
    load_mesh(
      model_asset->meshes.push(),
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
      model_asset, node->mChildren[idx], scene, transform, new_indices_pack
    );
  }
}


void Models::load_model_asset(Models::ModelAsset *model_asset) {
  // If we're not loading from a file, all the data has already
  // been loaded previously, so we just need to handle the
  // shaders and textures and so on, so skip this.
  char full_path[256]; // TODO: Fix unsafe strings?
  strcpy(full_path, MODEL_DIR);
  strcat(full_path, model_asset->path);

  if (model_asset->model_source == ModelSource::file) {
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
      model_asset, scene->mRootNode, scene, glm::mat4(1.0f), 0ULL
    );

    aiReleaseImport(scene);
  }

  model_asset->is_mesh_data_loading_in_progress = false;
  model_asset->is_mesh_data_loading_done = true;
}


void Models::copy_textures_to_pbo(
  Models::ModelAsset *model_asset,
  Materials::PersistentPbo *persistent_pbo
) {
  for (uint32 idx = 0; idx < model_asset->materials.size; idx++) {
    Materials::Material *material = model_asset->materials[idx];

    if (material->textures.size > 0) {
      Materials::copy_material_textures_to_pbo(
        material, persistent_pbo
      );
    }
  }

  model_asset->is_texture_copying_to_pbo_done = true;
  model_asset->is_texture_copying_to_pbo_in_progress = false;
}


void Models::bind_texture_uniforms_for_mesh(Models::Mesh *mesh) {
  Materials::Material *material = mesh->material;
  Shaders::ShaderAsset *shader_asset = material->shader_asset;

  if (shader_asset->type != Shaders::ShaderType::depth) {
    glUseProgram(shader_asset->program);

    for (
      uint32 uniform_idx = 0;
      uniform_idx < shader_asset->n_intrinsic_uniforms;
      uniform_idx++
    ) {
      const char *uniform_name = shader_asset->intrinsic_uniform_names[uniform_idx];
      if (strcmp(uniform_name, "should_use_normal_map") == 0) {
        Shaders::set_bool(
          shader_asset, "should_use_normal_map", material->should_use_normal_map
        );
      } else if (strcmp(uniform_name, "albedo_static") == 0) {
        Shaders::set_vec4(
          shader_asset, "albedo_static", &material->albedo_static
        );
      } else if (strcmp(uniform_name, "metallic_static") == 0) {
        Shaders::set_float(
          shader_asset, "metallic_static", material->metallic_static
        );
      } else if (strcmp(uniform_name, "roughness_static") == 0) {
        Shaders::set_float(
          shader_asset, "roughness_static", material->roughness_static
        );
      } else if (strcmp(uniform_name, "ao_static") == 0) {
        Shaders::set_float(
          shader_asset, "ao_static", material->ao_static
        );
      }
    }

    Shaders::reset_texture_units(shader_asset);

    for (uint32 idx = 0; idx < material->textures.size; idx++) {
      Materials::Texture *texture = material->textures[idx];
      const char *uniform_name = material->texture_uniform_names[idx];
#if 1
      log_info(
        "Setting uniforms: (uniform_name %s) "
        "(texture->texture_name %d)",
        uniform_name, texture->texture_name
      );
#endif
      Shaders::set_int(
        shader_asset,
        uniform_name,
        Shaders::add_texture_unit(shader_asset, texture->texture_name, texture->target)
      );
    }
  }

  shader_asset->did_set_texture_uniforms = true;
}


void Models::create_entities(
  Models::ModelAsset *model_asset,
  EntitySets::EntitySet *entity_set,
  EntitySets::DrawableComponentSet *drawable_component_set,
  EntitySets::SpatialComponentSet *spatial_component_set,
  EntitySets::LightComponentSet *light_component_set,
  EntitySets::BehaviorComponentSet *behavior_component_set
) {
  if (Entities::is_spatial_component_valid(&model_asset->spatial_component)) {
    Entities::SpatialComponent *spatial_component =
      EntitySets::get_spatial_component_from_set(
        spatial_component_set,
        model_asset->spatial_component.entity_handle
      );
    assert(spatial_component);
    *spatial_component = model_asset->spatial_component;
  }

  if (Entities::is_light_component_valid(&model_asset->light_component)) {
    Entities::LightComponent *light_component =
      EntitySets::get_light_component_from_set(
        light_component_set,
        model_asset->light_component.entity_handle
      );
    assert(light_component);
    *light_component = model_asset->light_component;
  }

  if (Entities::is_behavior_component_valid(&model_asset->behavior_component)) {
    Entities::BehaviorComponent *behavior_component =
      EntitySets::get_behavior_component_from_set(
        behavior_component_set,
        model_asset->behavior_component.entity_handle
      );
    assert(behavior_component);
    *behavior_component = model_asset->behavior_component;
  }

  if (model_asset->meshes.size == 1) {
    Entities::DrawableComponent *drawable_component =
      EntitySets::get_drawable_component_from_set(
        drawable_component_set,
        model_asset->entity_handle
      );
    assert(drawable_component);
    Entities::init_drawable_component(
      drawable_component,
      model_asset->entity_handle,
      model_asset->meshes[0],
      model_asset->render_pass
    );
  } else if (model_asset->meshes.size > 1) {
    for (uint32 idx = 0; idx < model_asset->meshes.size; idx++) {
      Models::Mesh *mesh = model_asset->meshes[idx];

      Entities::Entity *child_entity = EntitySets::add_entity_to_set(
        entity_set,
        model_asset->name
      );

      if (Entities::is_spatial_component_valid(&model_asset->spatial_component)) {
        Entities::SpatialComponent *spatial_component =
          EntitySets::get_spatial_component_from_set(
            spatial_component_set,
            child_entity->handle
          );
        assert(spatial_component);
        Entities::init_spatial_component(
          spatial_component,
          child_entity->handle,
          glm::vec3(0.0f),
          glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f)),
          glm::vec3(0.0f),
          model_asset->entity_handle
        );
      }

      Entities::DrawableComponent *drawable_component =
        EntitySets::get_drawable_component_from_set(
          drawable_component_set,
          child_entity->handle
        );
      assert(drawable_component);
      Entities::init_drawable_component(
        drawable_component,
        child_entity->handle,
        mesh,
        model_asset->render_pass
      );
    }
  }
}


void Models::prepare_for_draw(
  Models::ModelAsset *model_asset,
  Materials::PersistentPbo *persistent_pbo,
  Materials::TextureNamePool *texture_name_pool,
  Queue<Tasks::Task> *task_queue,
  EntitySets::EntitySet *entity_set,
  EntitySets::DrawableComponentSet *drawable_component_set,
  EntitySets::SpatialComponentSet *spatial_component_set,
  EntitySets::LightComponentSet *light_component_set,
  EntitySets::BehaviorComponentSet *behavior_component_set
) {
  // Step 1: Load mesh data. This is done on a separate thread.
  if (
    !model_asset->is_mesh_data_loading_in_progress &&
    !model_asset->is_mesh_data_loading_done
  ) {
    model_asset->is_mesh_data_loading_in_progress = true;
    task_queue->push({Tasks::TaskType::load_model, model_asset, nullptr});
  }

  // Step 2: Once the mesh data is loaded, set up vertex buffers for these meshes.
  if (
    model_asset->is_mesh_data_loading_done &&
    !model_asset->is_vertex_buffer_setup_done
  ) {
    for (uint32 idx = 0; idx < model_asset->meshes.size; idx++) {
      Models::Mesh *mesh = model_asset->meshes[idx];
      setup_mesh_vertex_buffers_for_file_source(mesh, &mesh->vertices, &mesh->indices);
    }
    model_asset->is_vertex_buffer_setup_done = true;
  }

  if (
    model_asset->is_mesh_data_loading_done &&
    model_asset->materials.size > 0
  ) {
    // Step 3: Once the mesh data is loaded, bind materials for their respective meshes.
    if (model_asset->is_mesh_data_loading_done && !model_asset->is_shader_setting_done) {
      for (
        uint32 idx_material = 0;
        idx_material < model_asset->materials.size;
        idx_material++
      ) {
        Materials::Material *material = model_asset->materials[idx_material];
        for (uint32 idx_mesh = 0; idx_mesh < model_asset->meshes.size; idx_mesh++) {
          Models::Mesh *mesh = model_asset->meshes[idx_mesh];
          uint8 mesh_number = pack_get(&mesh->indices_pack, 0);
          // For our model's mesh number `mesh_number`, we want to choose
          // material `idx_mesh` such that `mesh_number == idx_mesh`, i.e.
          // we choose the 4th material for mesh number 4.
          // However, if we have more meshes than materials, the extra
          // meshes all get material number 0.
          if (
            mesh_number == idx_material ||
            (mesh_number >= model_asset->materials.size && idx_material == 0)
          ) {
            mesh->material = material;
          }
        }
      }
      model_asset->is_shader_setting_done = true;
    }

    // Step 4: In parallel with the above, on a second thread, load all textures
    // for model_asset model from their files and copy them over to the PBO.
    if (
      !model_asset->is_texture_copying_to_pbo_done &&
      !model_asset->is_texture_copying_to_pbo_in_progress
    ) {
      // NOTE: Make sure we don't spawn threads that do nothing, or queue up
      // useless work for threads! This means only copying textures for meshes
      // that have one or more textures that do not have names yet.
      bool32 should_try_to_copy_textures = false;
      for (uint32 idx = 0; idx < model_asset->materials.size; idx++) {
        Materials::Material *material = model_asset->materials[idx];
        if (material->textures.size > 0) {
          for (
            uint32 texture_idx = 0;
            texture_idx < material->textures.size;
            texture_idx++
          ) {
            Materials::Texture *texture = material->textures[texture_idx];
            if (!texture->texture_name) {
              should_try_to_copy_textures = true;
              break;
            }
          }
        }
      }

      if (should_try_to_copy_textures) {
        model_asset->is_texture_copying_to_pbo_in_progress = true;
        task_queue->push({
          Tasks::TaskType::copy_textures_to_pbo, model_asset, persistent_pbo
        });
      } else {
        model_asset->is_texture_copying_to_pbo_done = true;
        model_asset->is_texture_creation_done = true;
      }
    }

    // Step 5: Once all of the above is complete, copy the texture data from the
    // PBO to the actual textures.
    if (
      model_asset->is_mesh_data_loading_done &&
      model_asset->is_vertex_buffer_setup_done &&
      model_asset->is_shader_setting_done &&
      model_asset->is_texture_copying_to_pbo_done &&
      !model_asset->is_texture_creation_done
    ) {
      START_TIMER(copy_pbo_to_texture);

      for (uint32 idx = 0; idx < model_asset->materials.size; idx++) {
        Materials::Material *material = model_asset->materials[idx];

        if (
          material->textures.size > 0 &&
          !material->have_textures_been_generated
        ) {
          Materials::generate_textures_from_pbo(
            material,
            persistent_pbo,
            texture_name_pool
          );
        }
      }

      model_asset->is_texture_creation_done = true;
      END_TIMER_MIN(copy_pbo_to_texture, 5);
    }

    // Step 6: Bind the texture uniforms.
    // NOTE: Because the shader might be reloaded at any time, we need to
    // check whether or not we need to set any uniforms every time.
    if (model_asset->is_texture_creation_done) {
      for (uint32 idx = 0; idx < model_asset->materials.size; idx++) {
        Materials::Material *material = model_asset->materials[idx];

        if (
          !material->shader_asset->did_set_texture_uniforms
        ) {
          for (uint32 idx_mesh = 0; idx_mesh < model_asset->meshes.size; idx_mesh++) {
            Models::Mesh *mesh = model_asset->meshes[idx_mesh];
            if (!mesh->material->shader_asset->did_set_texture_uniforms) {
              bind_texture_uniforms_for_mesh(mesh);
            }
          }
        }
      }
    }

    // Step 7: Create the entities.
    if (
      model_asset->is_texture_creation_done &&
      !model_asset->is_entity_creation_done
    ) {
      create_entities(
        model_asset,
        entity_set,
        drawable_component_set,
        spatial_component_set,
        light_component_set,
        behavior_component_set
      );
      model_asset->is_entity_creation_done = true;
    }

  }
}


Models::ModelAsset* Models::init_model_asset(
  ModelAsset *model_asset,
  MemoryPool *memory_pool,
  ModelSource model_source,
  const char *name,
  const char *path,
  Renderer::RenderPassFlag render_pass,
  Entities::EntityHandle entity_handle
) {
  strcpy(model_asset->name, name);
  model_asset->model_source = model_source;
  model_asset->render_pass = render_pass;
  model_asset->entity_handle = entity_handle;

  model_asset->meshes = Array<Models::Mesh>(
    memory_pool, MAX_N_MESHES, "meshes"
  );
  model_asset->materials = Array<Materials::Material>(
    memory_pool, MAX_N_MATERIALS, "materials"
  );

  strcpy(model_asset->path, path);
  // NOTE: We do not load ModelSource::file models here.
  // They will be loaded gradually in `::prepare_for_draw()`.

  return model_asset;
}


Models::ModelAsset* Models::init_model_asset(
  ModelAsset *model_asset,
  MemoryPool *memory_pool,
  ModelSource model_source,
  real32 *vertex_data, uint32 n_vertices,
  uint32 *index_data, uint32 n_indices,
  const char *name,
  GLenum mode,
  Renderer::RenderPassFlag render_pass,
  Entities::EntityHandle entity_handle
) {
  strcpy(model_asset->name, name);
  model_asset->model_source = model_source;
  model_asset->render_pass = render_pass;
  model_asset->entity_handle = entity_handle;

  model_asset->meshes = Array<Models::Mesh>(
    memory_pool, MAX_N_MESHES, "meshes"
  );
  model_asset->materials = Array<Materials::Material>(
    memory_pool, MAX_N_MATERIALS, "materials"
  );

  Models::Mesh *mesh = model_asset->meshes.push();
  mesh->transform = glm::mat4(1.0f);
  mesh->material = nullptr;
  mesh->mode = mode;
  mesh->n_vertices = n_vertices;
  mesh->n_indices = n_indices;
  mesh->indices_pack = 0UL;

  setup_mesh_vertex_buffers_for_data_source(
    mesh, vertex_data, n_vertices, index_data, n_indices
  );
  model_asset->is_mesh_data_loading_done = true;
  model_asset->is_vertex_buffer_setup_done = true;

  return model_asset;
}
