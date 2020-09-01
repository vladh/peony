#define _CRT_SECURE_NO_WARNINGS

#include "models.hpp"

#include <string.h>

#include <glad/glad.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "log.hpp"
#include "util.hpp"


uint32 models_load_texture_from_file(const char *directory, const char *filename) {
  char path[128];
  strcpy(path, directory);
  strcat(path, "/");
  strcat(path, filename);

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

    /* stbi_image_free(data); */
  } else {
    log_error("Texture failed to load at path: %s", path);
    /* stbi_image_free(data); */
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
    GL_ARRAY_BUFFER, sizeof(mesh->vertices), mesh->vertices, GL_STATIC_DRAW
  );

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER, sizeof(mesh->indices), mesh->indices, GL_STATIC_DRAW
  );

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
    0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0
  );

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
    1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
    (void*)offsetof(Vertex, normal)
  );

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(
    2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
    (void*)offsetof(Vertex, tex_coords)
  );
}

void models_load_mesh(Model *model, Mesh *mesh, aiMesh *mesh_data, const aiScene *scene) {
  mesh->n_vertices = 0;
  mesh->n_indices = 0;
  mesh->n_textures = 0;

  // Load vertices
  for (uint32 idx = 0; idx < mesh_data -> mNumVertices; idx++) {
    Vertex vertex;

    glm::vec3 position;
    position.x = mesh_data->mVertices[idx].x;
    position.y = mesh_data->mVertices[idx].y;
    position.z = mesh_data->mVertices[idx].z;
    vertex.position = position;

    glm::vec3 normal;
    normal.x = mesh_data->mNormals[idx].x;
    normal.y = mesh_data->mNormals[idx].y;
    normal.z = mesh_data->mNormals[idx].z;
    vertex.normal = normal;

    if (mesh_data->mTextureCoords[0]) {
      glm::vec2 tex_coords;
      tex_coords.x = mesh_data->mTextureCoords[0][idx].x;
      tex_coords.y = mesh_data->mTextureCoords[0][idx].y;
      vertex.tex_coords = tex_coords;
    } else {
      vertex.tex_coords = glm::vec2(0.0f, 0.0f);
    }

    /* log_info("Loading vertex %d", mesh->n_vertices); */
    mesh->vertices[mesh->n_vertices++] = vertex;
  }

  // Load indices
  for (uint32 idx_face = 0; idx_face < mesh_data->mNumFaces; idx_face++) {
    aiFace face = mesh_data->mFaces[idx_face];
    for (uint32 idx_index = 0; idx_index < face.mNumIndices; idx_index++) {
      /* log_info("Loading index %d", mesh->n_indices); */
      mesh->indices[mesh->n_indices++] = face.mIndices[idx_index];
    }
  }

  // Load textures
  aiMaterial *material = scene->mMaterials[mesh_data->mMaterialIndex];

  aiTextureType texture_type = aiTextureType_DIFFUSE;
  const char* texture_type_name = "";

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
      const char* texture_filename = texture_filename_aistring.C_Str();

      bool32 should_skip = false;
      for (uint32 idx_loaded = 0; idx_loaded < mesh->n_textures; idx_loaded++) {
        if (strcmp(mesh->textures[idx_loaded].filename, texture_filename) == 0) {
          mesh->textures[mesh->n_textures++] = mesh->textures[idx_loaded];
          should_skip = true;
          break;
        }
      }

      if (!should_skip) {
        Texture texture;
        texture.id = models_load_texture_from_file(model->directory, texture_filename);
        texture.type = texture_type_name;
        texture.filename = texture_filename;
        /* log_info("Loading texture %d", mesh->n_textures); */
        mesh->textures[mesh->n_textures++] = texture;
      }
    }
  }

  models_setup_mesh(mesh);
}

void models_draw_mesh(Mesh *mesh, uint32 shader_program) {
  uint32 diffuse_idx = 1;
  uint32 specular_idx = 1;

  for (uint32 idx = 0; idx < mesh->n_textures; idx++) {
    /* log_info("Texture %d", idx); */
    glActiveTexture(GL_TEXTURE0 + idx);

    char idx_str[2];
    char uniform_name[128];
    const char* texture_type = mesh->textures[idx].type;

    if (strcmp(texture_type, "texture_diffuse") == 0) {
      sprintf(idx_str, "%d", diffuse_idx);
      /* strcpy(uniform_name, "material."); */
      strcpy(uniform_name, "");
      strcat(uniform_name, texture_type);
      strcat(uniform_name, idx_str);
      diffuse_idx++;
    } else if (strcmp(texture_type, "texture_specular") == 0) {
      sprintf(idx_str, "%d", specular_idx);
      /* strcpy(uniform_name, "material."); */
      strcpy(uniform_name, "");
      strcat(uniform_name, texture_type);
      strcat(uniform_name, idx_str);
      specular_idx++;
    }

    /* log_info("uniform_name %s", uniform_name); */
    glUniform1f(
      glGetUniformLocation(shader_program, uniform_name),
      (real32)idx
    );

    glBindTexture(GL_TEXTURE_2D, mesh->textures[idx].id);
  }

  glActiveTexture(GL_TEXTURE0);

  glBindVertexArray(mesh->vao);
  glDrawElements(GL_TRIANGLES, mesh->n_indices, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}

void models_draw_model(Model *model, uint32 shader_program) {
  for (uint32 idx = 0; idx < model->n_meshes; idx++) {
    /* log_info("Drawing mesh %d", idx); */
    models_draw_mesh(&model->meshes[idx], shader_program);
  }
}


void models_load_model_node(Model *model, aiNode *node, const aiScene *scene) {
  for (uint32 idx = 0; idx < node->mNumMeshes; idx++) {
    aiMesh *mesh_data = scene->mMeshes[node->mMeshes[idx]];
    log_info("Loading mesh %d", model->n_meshes);
    models_load_mesh(model, &model->meshes[model->n_meshes++], mesh_data, scene);
  }

  for (uint32 idx = 0; idx < node->mNumChildren; idx++) {
    models_load_model_node(model, node->mChildren[idx], scene);
  }
}

void models_load_model(Model *model, const char* directory, const char* filename) {
  char path[128];
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

  model->n_meshes = 0;
  model->directory = directory;

  models_load_model_node(model, scene->mRootNode, scene);
}
