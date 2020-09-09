#define _CRT_SECURE_NO_WARNINGS

#include "models.hpp"

#include <string.h>

#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "memory.hpp"
#include "shader.hpp"
#include "array.hpp"
#include "log.hpp"
#include "util.hpp"


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

void models_setup_mesh(Mesh *mesh, Shader shader) {
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

  location = glGetAttribLocation(shader.program, "position");
  glEnableVertexAttribArray(location);
  glVertexAttribPointer(
    location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0
  );

  location = glGetAttribLocation(shader.program, "normal");
  glEnableVertexAttribArray(location);
  glVertexAttribPointer(
    location, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal)
  );

  location = glGetAttribLocation(shader.program, "tex_coords");
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
  log_info("Pushing memory for vertices");
  mesh->vertices.items = (Vertex*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(Vertex) * mesh->vertices.max_size
  );

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
  log_info("Pushing memory for indices");
  mesh->indices.items = (uint32*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(uint32) * mesh->indices.max_size
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
  aiMaterial *material = scene->mMaterials[mesh_data->mMaterialIndex];

  uint32 n_diffuse_textures = material->GetTextureCount(aiTextureType_DIFFUSE);
  uint32 n_specular_textures = material->GetTextureCount(aiTextureType_SPECULAR);

  mesh->textures.size = 0;
  mesh->textures.max_size = n_diffuse_textures + n_specular_textures;
  log_info("Pushing memory for textures");
  mesh->textures.items = (Texture*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(Texture) * mesh->textures.max_size
  );

  aiTextureType texture_type = aiTextureType_DIFFUSE;
  const char *texture_type_name = "";

  for (uint32 idx_type = 0; idx_type < 2; idx_type++) {
    if (idx_type == 0) {
      texture_type = aiTextureType_DIFFUSE;
      texture_type_name = "texture_diffuse";
    } else if (idx_type == 1) {
      texture_type = aiTextureType_SPECULAR;
      texture_type_name = "texture_specular";
    }

    for (uint32 idx = 0; idx < material->GetTextureCount(texture_type); idx++) {
      aiString texture_filename_aistring;
      material->GetTexture(texture_type, idx, &texture_filename_aistring);
      const char *texture_filename = texture_filename_aistring.C_Str();

      bool32 should_skip = false;
      for (uint32 idx_loaded = 0; idx_loaded < mesh->textures.size; idx_loaded++) {
        if (strcmp(mesh->textures.items[idx_loaded].filename, texture_filename) == 0) {
          // TODO: Change this to pointers so we don't copy?
          array_push(&mesh->textures, mesh->textures.items[idx_loaded]);
          should_skip = true;
          break;
        }
      }

      if (!should_skip) {
        char texture_path[128];
        strcpy(texture_path, model->directory);
        strcat(texture_path, "/");
        strcat(texture_path, texture_filename);

        Texture *texture = (Texture*)array_push<Texture>(&mesh->textures);
        texture->id = models_load_texture_from_file(texture_path);
        texture->type = texture_type_name;
        texture->was_loaded_from_file = true;
        texture->filename = texture_filename;
      }
    }
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
  models_setup_mesh(mesh, model->shader_asset->shader);
}

void models_load_model_node(
  Memory *memory, Model *model,
  aiNode *node, const aiScene *scene
) {
  for (uint32 idx = 0; idx < node->mNumMeshes; idx++) {
    aiMesh *mesh_data = scene->mMeshes[node->mMeshes[idx]];
    log_info("Loading mesh %d", model->meshes.size);
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
  strcpy(path, directory);
  strcat(path, "/");
  strcat(path, filename);

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
  log_info("Pushing memory for meshes");
  model->meshes.items = (Mesh*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(Mesh) * model->meshes.max_size
  );

  models_load_model_node(memory, model, scene->mRootNode, scene);
}

ModelAsset* models_make_asset_from_file(
  Memory *memory, ModelAsset *model_asset, ShaderAsset *shader_asset,
  const char *name, const char *directory, const char *filename
) {
  model_asset->info.name = name;
  model_asset->model.shader_asset = shader_asset;
  model_asset->model.should_load_textures_from_file = true;
  model_asset->model.directory = directory;
  models_load_model(
    memory, &model_asset->model, directory, filename
  );
  return model_asset;
}

ModelAsset* models_make_asset_from_data(
  Memory *memory, ModelAsset *model_asset,
  ShaderAsset *shader_asset,
  real32 *vertex_data, uint32 n_vertices,
  real32 *index_data, uint32 n_indices,
  const char *name, const char *texture_path,
  uint32 mode
) {
  model_asset->info.name = name;

  Model *model = &model_asset->model;

  model->shader_asset = shader_asset;
  model->should_load_textures_from_file = false;
  model->directory = "";

  model->meshes.size = 0;
  model->meshes.max_size = 1;
  log_info("Pushing memory for meshes");
  model->meshes.items = (Mesh*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(Mesh) * model->meshes.max_size
  );

  Mesh *mesh = model->meshes.items;
  model->meshes.size++;

  model->meshes.items[0].mode = mode;

  // Vertices
  mesh->vertices.size = 0;
  mesh->vertices.max_size = n_vertices;
  log_info("Pushing memory for vertices");
  mesh->vertices.items = (Vertex*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(Vertex) * mesh->vertices.max_size
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

  mesh->indices.size = 0;
  mesh->indices.max_size = n_indices;
  log_info("Pushing memory for indices");
  mesh->indices.items = (uint32*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(uint32) * mesh->indices.max_size
  );

  memcpy(index_data, mesh->indices.items, sizeof(uint32) * n_indices);
  mesh->indices.size = n_indices;

  // Textures
  mesh->textures.size = 0;
  mesh->textures.max_size = 1;
  log_info("Pushing memory for textures");
  mesh->textures.items = (Texture*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(Texture) * mesh->textures.max_size
  );
  Texture *texture = (Texture*)array_push<Texture>(&mesh->textures);

  if (strcmp(texture_path, "") == 0) {
    texture->id = -1;
    texture->type = "";
    texture->was_loaded_from_file = false;
    texture->filename = "";
  } else {
    texture->id = models_load_texture_from_file(texture_path);
    texture->type = "texture_diffuse";
    texture->was_loaded_from_file = true;
    texture->filename = "";
  }

  models_setup_mesh(mesh, shader_asset->shader);

  return model_asset;
}

void models_draw_mesh(Mesh *mesh, uint32 shader_program) {
  uint32 diffuse_idx = 1;
  uint32 specular_idx = 1;

  for (uint32 idx = 0; idx < mesh->textures.size; idx++) {
    glActiveTexture(GL_TEXTURE0 + idx);

    char idx_str[2];
    char uniform_name[128];
    const char *texture_type = mesh->textures.items[idx].type;

    if (strcmp(texture_type, "texture_diffuse") == 0) {
      sprintf(idx_str, "%d", diffuse_idx);
      strcpy(uniform_name, "");
      strcat(uniform_name, texture_type);
      strcat(uniform_name, idx_str);
      diffuse_idx++;
    } else if (strcmp(texture_type, "texture_specular") == 0) {
      sprintf(idx_str, "%d", specular_idx);
      strcpy(uniform_name, "");
      strcat(uniform_name, texture_type);
      strcat(uniform_name, idx_str);
      specular_idx++;
    }

    shader_set_float(shader_program, uniform_name, (real32)idx);

    glBindTexture(GL_TEXTURE_2D, mesh->textures.items[idx].id);
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
