#ifndef MODELS_H
#define MODELS_H

#include <glm/glm.hpp>
#include <assimp/scene.h>

#include "types.hpp"


typedef struct Vertex {
  glm::vec3 position;
  glm::vec3 normal;
  glm::vec2 tex_coords;
} Vertex;

typedef struct Texture {
  uint32 id;
  const char* type;
  const char* filename;
} Texture;

typedef struct Mesh {
  Vertex vertices[2048];
  uint32 n_vertices;
  uint32 indices[2048];
  uint32 n_indices;
  Texture textures[2048];
  uint32 n_textures;
  uint32 vao;
  uint32 vbo;
  uint32 ebo;
} Mesh;

typedef struct Model {
  Mesh meshes[128];
  uint32 n_meshes;
  const char* directory;
} Model;

uint32 models_load_texture_from_file(const char *filename, const char *directory);
void models_setup_mesh(Mesh *mesh);
void models_init_mesh(
  Mesh *mesh,
  Vertex *vertices,
  uint32 *indices,
  Texture *textures
);
void models_load_mesh(Model *model, Mesh *mesh, aiMesh *mesh_data, const aiScene *scene);
void models_draw_mesh(Mesh *mesh, uint32 shader_program);
void models_draw_model(Model *model, uint32 shader_program);
void models_load_model_node(Model *model, aiNode *node, const aiScene *scene);
void models_load_model(Model *model, const char* directory, const char* filename);

#endif
