void shader_assert_shader_status_ok(uint32 shader) {
  int32 status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if (status != 1) {
    char message[512];
    glGetShaderInfoLog(shader, 512, NULL, message);
    log_info("Compiling shader %d: (status %d) (message %s)", shader, status, message);
    log_error("Shader compilation failed");
    exit(EXIT_FAILURE);
  }
}

void shader_assert_program_status_ok(uint32 program) {
  int32 status;
  glGetProgramiv(program, GL_LINK_STATUS, &status);

  if (status != 1) {
    char message[512];
    glGetProgramInfoLog(program, 512, NULL, message);
    log_info("Compiling program %d: (status %d) (message %s)", program, status, message);
    log_error("Program loading failed");
    exit(EXIT_FAILURE);
  }
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

void shader_set_int(uint32 program, const char *name, uint32 value) {
  glUniform1i(glGetUniformLocation(program, name), value);
}

void shader_set_bool(uint32 program, const char *name, bool value) {
  shader_set_int(program, name, (uint32)value);
}

void shader_set_float(uint32 program, const char *name, float value) {
  glUniform1f(glGetUniformLocation(program, name), value);
}

void shader_set_vec2(uint32 program, const char *name, glm::vec2 *value) {
  glUniform2fv(glGetUniformLocation(program, name), 1, glm::value_ptr(*value));
}

void shader_set_vec3(uint32 program, const char *name, glm::vec3 *value) {
  glUniform3fv(glGetUniformLocation(program, name), 1, glm::value_ptr(*value));
}

void shader_set_vec4(uint32 program, const char *name, glm::vec4 *value) {
  glUniform4fv(glGetUniformLocation(program, name), 1, glm::value_ptr(*value));
}

void shader_set_mat2(uint32 program, const char *name, glm::mat2 *mat) {
  glUniformMatrix2fv(glGetUniformLocation(program, name), 1, GL_FALSE, glm::value_ptr(*mat));
}

void shader_set_mat3(uint32 program, const char *name, glm::mat3 *mat) {
  glUniformMatrix3fv(glGetUniformLocation(program, name), 1, GL_FALSE, glm::value_ptr(*mat));
}

void shader_set_mat4(uint32 program, const char *name, glm::mat4 *mat) {
  glUniformMatrix4fv(glGetUniformLocation(program, name), 1, GL_FALSE, glm::value_ptr(*mat));
}
