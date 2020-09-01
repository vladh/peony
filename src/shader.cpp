#include <cstdlib>

#include <glad/glad.h>

#include "types.hpp"
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
  glGetProgramiv(shader, GL_COMPILE_STATUS, &status);

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

uint32 shader_load(const char* source, GLenum shaderType) {
  uint32 shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  shader_assert_shader_status_ok(shader);
  return shader;
}

uint32 shader_make_program(uint32 vertexShader, uint32 fragmentShader) {
  uint32 shader_program = glCreateProgram();
  glAttachShader(shader_program, vertexShader);
  glAttachShader(shader_program, fragmentShader);
  glLinkProgram(shader_program);
  shader_assert_program_status_ok(shader_program);
  return shader_program;
}

uint32 shader_make_program_with_paths(const char* vertPath, const char* fragPath) {
  return shader_make_program(
    shader_load(util_load_file(vertPath), GL_VERTEX_SHADER),
    shader_load(util_load_file(fragPath), GL_FRAGMENT_SHADER)
  );
}
