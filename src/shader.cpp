#include <cstdlib>

#include <glad/glad.h>

#include "memory.hpp"
#include "types.hpp"
#include "asset.hpp"
#include "log.hpp"
#include "util.hpp"


void shader_assert_shader_status_ok(uint32 shader) {
  int32 status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  char message[512];
  glGetShaderInfoLog(shader, 512, NULL, message);

  log_info("[assertShaderStatusOk] Compilation for shader %d", shader);
  log_info("Status: %d", status);
  log_info("Message: %s", message);

  if (status != 1) {
    log_error("[assertShaderStatusOk] Shader compilation failed");
    exit(EXIT_FAILURE);
  }

  log_newline();
}

void shader_assert_program_status_ok(uint32 shader) {
  int32 status;
  glGetProgramiv(shader, GL_LINK_STATUS, &status);

  char message[512];
  glGetProgramInfoLog(shader, 512, NULL, message);

  log_info("[assertProgramStatusOk] Loading program");
  log_info("Status: %d", status);
  log_info("Message: %s", message);

  if (status != 1) {
    log_error("[assertProgramStatusOk] Program loading failed");
    exit(EXIT_FAILURE);
  }

  log_newline();
}

uint32 shader_load(const char *source, GLenum shader_type) {
  uint32 shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  shader_assert_shader_status_ok(shader);
  return shader;
}

uint32 shader_make_program(uint32 vertex_shader, uint32 fragment_shader) {
  uint32 shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  shader_assert_program_status_ok(shader_program);
  return shader_program;
}

uint32 shader_make_program_with_paths(const char *vert_path, const char *frag_path) {
  return shader_make_program(
    shader_load(util_load_file(vert_path), GL_VERTEX_SHADER),
    shader_load(util_load_file(frag_path), GL_FRAGMENT_SHADER)
  );
}

ShaderAsset* shader_make_asset(
  ShaderAsset *asset, const char *name,
  const char *vertex_path, const char* frag_path
) {
  uint32 program = shader_make_program_with_paths(vertex_path, frag_path);
  asset->info.name = name;
  asset->shader.program = program;
  return asset;
}

void shader_set_int(uint32 program, const char *name, float value) {
  glUniform1i(glGetUniformLocation(program, name), value);
}

void shader_set_bool(uint32 program, const char *name, float value) {
  shader_set_int(program, name, (int)value);
}

void shader_set_float(uint32 program, const char *name, float value) {
  glUniform1f(glGetUniformLocation(program, name), value);
}

void shader_set_vec2(uint32 program, const char *name, const glm::vec2 &value) {
  glUniform2fv(glGetUniformLocation(program, name), 1, &value[0]);
}

void shader_set_vec3(uint32 program, const char *name, const glm::vec3 &value) {
  glUniform3fv(glGetUniformLocation(program, name), 1, &value[0]);
}

void shader_set_vec4(uint32 program, const char *name, const glm::vec4 &value) {
  glUniform4fv(glGetUniformLocation(program, name), 1, &value[0]);
}

void shader_set_mat2(uint32 program, const char *name, const glm::mat2 &mat) {
  glUniformMatrix2fv(glGetUniformLocation(program, name), 1, GL_FALSE, &mat[0][0]);
}

void shader_set_mat3(uint32 program, const char *name, const glm::mat3 &mat) {
  glUniformMatrix3fv(glGetUniformLocation(program, name), 1, GL_FALSE, &mat[0][0]);
}

void shader_set_mat4(uint32 program, const char *name, const glm::mat4 &mat) {
  glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, &mat[0][0]);
}
