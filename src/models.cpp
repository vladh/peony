void Models::setup_mesh_vertex_buffers_for_data_source(
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
  assert(vertex_data && n_vertices > 0);

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
  Mesh *mesh, Array<Vertex> *vertices, Array<uint32> *indices
) {
  assert(vertices->size > 0);

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
  Mesh *mesh, aiMesh *mesh_data, const aiScene *scene,
  glm::mat4 transform, Pack indices_pack
) {
  mesh->transform = transform;
  glm::mat3 normal_matrix = glm::mat3(glm::transpose(glm::inverse(transform)));
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
  EntityLoader *entity_loader,
  aiNode *node, const aiScene *scene,
  glm::mat4 accumulated_transform, Pack indices_pack
) {
  glm::mat4 node_transform = Util::aimatrix4x4_to_glm(&node->mTransformation);
  glm::mat4 transform = accumulated_transform * node_transform;

  for (uint32 idx = 0; idx < node->mNumMeshes; idx++) {
    aiMesh *mesh_data = scene->mMeshes[node->mMeshes[idx]];
    load_mesh(
      entity_loader->meshes.push(),
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
      entity_loader, node->mChildren[idx], scene, transform, new_indices_pack
    );
  }
}


void Models::load_model(EntityLoader *entity_loader) {
  // If we're not loading from a file, all the data has already
  // been loaded previously, so we just need to handle the
  // shaders and textures and so on, so skip this.
  char full_path[MAX_PATH];
  strcpy(full_path, MODEL_DIR);
  strcat(full_path, entity_loader->path);

  if (entity_loader->model_source == ModelSource::file) {
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
      entity_loader, scene->mRootNode, scene, glm::mat4(1.0f), 0ULL
    );

    aiReleaseImport(scene);
  }

  entity_loader->state = EntityLoaderState::mesh_data_loaded;
}


void Models::create_entities(
  EntityLoader *entity_loader,
  EntitySet *entity_set,
  DrawableComponentSet *drawable_component_set,
  SpatialComponentSet *spatial_component_set,
  LightComponentSet *light_component_set,
  BehaviorComponentSet *behavior_component_set
) {
  if (Entities::is_spatial_component_valid(&entity_loader->spatial_component)) {
    SpatialComponent *spatial_component = spatial_component_set->components.get(
      entity_loader->spatial_component.entity_handle
    );
    assert(spatial_component);
    *spatial_component = entity_loader->spatial_component;
  }

  if (Entities::is_light_component_valid(&entity_loader->light_component)) {
    LightComponent *light_component = light_component_set->components.get(
      entity_loader->light_component.entity_handle
    );
    assert(light_component);
    *light_component = entity_loader->light_component;
  }

  if (Entities::is_behavior_component_valid(&entity_loader->behavior_component)) {
    BehaviorComponent *behavior_component = behavior_component_set->components.get(
      entity_loader->behavior_component.entity_handle
    );
    assert(behavior_component);
    *behavior_component = entity_loader->behavior_component;
  }

  if (entity_loader->meshes.size == 1) {
    DrawableComponent *drawable_component = drawable_component_set->components.get(
      entity_loader->entity_handle
    );
    assert(drawable_component);
    *drawable_component = {
      .entity_handle = entity_loader->entity_handle,
      .mesh = entity_loader->meshes[0],
      .target_render_pass = entity_loader->render_pass,
    };
  } else if (entity_loader->meshes.size > 1) {
    for (uint32 idx = 0; idx < entity_loader->meshes.size; idx++) {
      Mesh *mesh = entity_loader->meshes[idx];

      Entity *child_entity = EntitySets::add_entity_to_set(
        entity_set,
        entity_loader->name
      );

      if (Entities::is_spatial_component_valid(&entity_loader->spatial_component)) {
        SpatialComponent *spatial_component = spatial_component_set->components.get(
          child_entity->handle
        );
        assert(spatial_component);
        *spatial_component = {
          .entity_handle = child_entity->handle,
          .position = glm::vec3(0.0f),
          .rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(0.0f)),
          .scale = glm::vec3(0.0f),
          .parent_entity_handle = entity_loader->entity_handle,
        };
      }

      DrawableComponent *drawable_component = drawable_component_set->components.get(
        child_entity->handle
      );
      assert(drawable_component);
      *drawable_component = {
        .entity_handle = child_entity->handle,
        .mesh = mesh,
        .target_render_pass = entity_loader->render_pass,
      };
    }
  }
}


bool32 Models::prepare_model_and_check_if_done(
  EntityLoader *entity_loader,
  PersistentPbo *persistent_pbo,
  TextureNamePool *texture_name_pool,
  Queue<Task> *task_queue,
  EntitySet *entity_set,
  DrawableComponentSet *drawable_component_set,
  SpatialComponentSet *spatial_component_set,
  LightComponentSet *light_component_set,
  BehaviorComponentSet *behavior_component_set
) {
  if (entity_loader->state == EntityLoaderState::initialized) {
    task_queue->push({
      .type = TaskType::load_model,
      .target = {
        .entity_loader = entity_loader,
      },
      .persistent_pbo = nullptr,
    });
    entity_loader->state = EntityLoaderState::mesh_data_being_loaded;
  }

  if (entity_loader->state == EntityLoaderState::mesh_data_being_loaded) {
    // Wait. The task will progress this for us.
  }

  if (entity_loader->state == EntityLoaderState::mesh_data_loaded) {
    // Setup vertex buffers
    if (entity_loader->model_source == ModelSource::file) {
      for (uint32 idx = 0; idx < entity_loader->meshes.size; idx++) {
        Mesh *mesh = entity_loader->meshes[idx];
        setup_mesh_vertex_buffers_for_file_source(mesh, &mesh->vertices, &mesh->indices);
      }
    }
    entity_loader->state = EntityLoaderState::vertex_buffers_set_up;
  }

  if (entity_loader->state == EntityLoaderState::vertex_buffers_set_up) {
    // Set material names for each mesh
    for (
      uint32 idx_material = 0; idx_material < entity_loader->n_materials; idx_material++
    ) {
      for (uint32 idx_mesh = 0; idx_mesh < entity_loader->meshes.size; idx_mesh++) {
        Mesh *mesh = entity_loader->meshes[idx_mesh];
        uint8 mesh_number = pack_get(&mesh->indices_pack, 0);
        // For our model's mesh number `mesh_number`, we want to choose
        // material `idx_mesh` such that `mesh_number == idx_mesh`, i.e.
        // we choose the 4th material for mesh number 4.
        // However, if we have more meshes than materials, the extra
        // meshes all get material number 0.
        if (
          mesh_number == idx_material ||
          (mesh_number >= entity_loader->n_materials && idx_material == 0)
        ) {
          strcpy(mesh->material_name, entity_loader->material_names[idx_material]);
        }
      }
    }

    // Create any entities we might need to create
    create_entities(
      entity_loader,
      entity_set,
      drawable_component_set,
      spatial_component_set,
      light_component_set,
      behavior_component_set
    );

    entity_loader->state = EntityLoaderState::complete;
  }

  if (entity_loader->state == EntityLoaderState::complete) {
    return true;
  }

  return false;
}


EntityLoader* Models::init_entity_loader(
  EntityLoader *entity_loader,
  MemoryPool *memory_pool,
  ModelSource model_source,
  const char *name,
  const char *path,
  RenderPassFlag render_pass,
  EntityHandle entity_handle
) {
  assert(entity_loader);
  strcpy(entity_loader->name, name);
  entity_loader->state = EntityLoaderState::initialized;
  entity_loader->model_source = model_source;
  entity_loader->render_pass = render_pass;
  entity_loader->entity_handle = entity_handle;

  entity_loader->meshes = Array<Mesh>(
    memory_pool, MAX_N_MESHES, "meshes"
  );

  strcpy(entity_loader->path, path);
  // NOTE: We do not load ModelSource::file models here.
  // They will be loaded gradually in `::prepare_model_and_check_if_done()`.

  return entity_loader;
}


EntityLoader* Models::init_entity_loader(
  EntityLoader *entity_loader,
  MemoryPool *memory_pool,
  ModelSource model_source,
  real32 *vertex_data, uint32 n_vertices,
  uint32 *index_data, uint32 n_indices,
  const char *name,
  GLenum mode,
  RenderPassFlag render_pass,
  EntityHandle entity_handle
) {
  assert(entity_loader);
  strcpy(entity_loader->name, name);
  entity_loader->model_source = model_source;
  entity_loader->render_pass = render_pass;
  entity_loader->entity_handle = entity_handle;

  entity_loader->meshes = Array<Mesh>(
    memory_pool, MAX_N_MESHES, "meshes"
  );

  Mesh *mesh = entity_loader->meshes.push();
  mesh->transform = glm::mat4(1.0f);
  mesh->mode = mode;
  mesh->n_vertices = n_vertices;
  mesh->n_indices = n_indices;
  mesh->indices_pack = 0UL;

  setup_mesh_vertex_buffers_for_data_source(
    mesh, vertex_data, n_vertices, index_data, n_indices
  );
  entity_loader->state = EntityLoaderState::vertex_buffers_set_up;

  return entity_loader;
}
