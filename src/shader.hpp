#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include "types.hpp"


typedef struct Shader {
  uint32 program;
} Shader;

typedef struct ShaderAsset ShaderAsset;

void shader_assert_shader_status_ok(uint32 shader);
void shader_assert_program_status_ok(uint32 shader);
uint32 shader_load(const char *source, GLenum shaderType);
uint32 shader_make_program(uint32 vertexShader, uint32 fragmentShader);
uint32 shader_make_program_with_paths(const char *vertPath, const char *fragPath);
ShaderAsset* shader_make_asset(
  ShaderAsset *asset, const char *name,
  const char *vertex_path, const char* frag_path
);

#endif
