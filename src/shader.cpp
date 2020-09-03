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
  State *state, const char *name,
  const char *vertex_path, const char* frag_path
) {
  uint32 program = shader_make_program_with_paths(vertex_path, frag_path);

  assert(state->n_shader_assets < state->max_n_shader_assets);
  ShaderAsset *asset = state->shader_assets + state->n_shader_assets++;

  asset->info.name = name;
  asset->shader.program = program;

  return asset;
}
