void shader_assert_shader_status_ok(uint32 shader, const char* path) {
  int32 status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  if (status != 1) {
    char message[512];
    glGetShaderInfoLog(shader, 512, NULL, message);
    log_info("Compiling shader %s: (status %d) (message %s)", path, status, message);
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

uint32 shader_load(const char *path, const char *source, GLenum shader_type) {
  uint32 shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  shader_assert_shader_status_ok(shader, path);
  return shader;
}

void shader_init(Shader *shader, uint32 program) {
  shader->program = program;
  // TODO: Is there a better way to do this?
  for (uint16 idx = 0; idx < LEN(shader->uniform_locations); idx++) {
    shader->uniform_locations[idx] = -1;
  }
}

void shader_init_program(uint32 shader_program) {
  uint32 uniform_block_index = glGetUniformBlockIndex(shader_program, "shader_common");
  glUniformBlockBinding(shader_program, uniform_block_index, 0);
}

uint32 shader_make_program(uint32 vertex_shader, uint32 fragment_shader) {
  uint32 shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  shader_assert_program_status_ok(shader_program);
  shader_init_program(shader_program);
  return shader_program;
}

uint32 shader_make_program(
  uint32 vertex_shader, uint32 fragment_shader, uint32 geometry_shader
) {
  uint32 shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glAttachShader(shader_program, geometry_shader);
  glLinkProgram(shader_program);
  shader_assert_program_status_ok(shader_program);
  shader_init_program(shader_program);
  return shader_program;
}

const char* shader_load_file(Memory *memory, const char *path) {
  // TODO: It's a bit wasteful to add `common.glsl` to every single
  // shader. Maybe it would be better to somehow do it selectively.
  return util_load_two_files(
    &memory->temp_memory_pool, SHADER_COMMON_PATH, path
  );
}

uint32 shader_make_program_with_paths(
  Memory *memory, const char *vert_path, const char *frag_path
) {
  return shader_make_program(
    shader_load(vert_path, shader_load_file(memory, vert_path), GL_VERTEX_SHADER),
    shader_load(frag_path, shader_load_file(memory, frag_path), GL_FRAGMENT_SHADER)
  );
}

uint32 shader_make_program_with_paths(
  Memory *memory, const char *vert_path, const char *frag_path, const char *geom_path
) {
  uint32 shader_program = shader_make_program(
    shader_load(vert_path, shader_load_file(memory, vert_path), GL_VERTEX_SHADER),
    shader_load(frag_path, shader_load_file(memory, frag_path), GL_FRAGMENT_SHADER),
    shader_load(geom_path, shader_load_file(memory, geom_path), GL_GEOMETRY_SHADER)
  );
  return shader_program;
}

ShaderAsset* shader_make_asset(
  ShaderAsset *asset, Memory *memory, const char *name,
  const char *vertex_path, const char* frag_path
) {
  uint32 program = shader_make_program_with_paths(
    memory, vertex_path, frag_path
  );
  asset->info.name = name;

  shader_init(&asset->shader, program);

  memory_reset_pool(&memory->temp_memory_pool);

  return asset;
}

ShaderAsset* shader_make_asset(
  ShaderAsset *asset, Memory *memory, const char *name,
  const char *vertex_path, const char *frag_path, const char *geom_path
) {
  uint32 program = shader_make_program_with_paths(
    memory, vertex_path, frag_path, geom_path
  );
  asset->info.name = name;

  shader_init(&asset->shader, program);

  return asset;
}

uint32 shader_get_uniform_location(Shader *shader, const char *name) {
  return glGetUniformLocation(shader->program, name);
}

void shader_set_int(Shader *shader, const char *name, uint32 value) {
  glUniform1i(shader_get_uniform_location(shader, name), value);
}

void shader_set_bool(Shader *shader, const char *name, bool value) {
  shader_set_int(shader, name, (uint32)value);
}

void shader_set_float(Shader *shader, const char *name, float value) {
  glUniform1f(shader_get_uniform_location(shader, name), value);
}

void shader_set_vec2(Shader *shader, const char *name, glm::vec2 *value) {
  glUniform2fv(shader_get_uniform_location(shader, name), 1, glm::value_ptr(*value));
}

void shader_set_vec3(Shader *shader, const char *name, glm::vec3 *value) {
  glUniform3fv(shader_get_uniform_location(shader, name), 1, glm::value_ptr(*value));
}

void shader_set_vec4(Shader *shader, const char *name, glm::vec4 *value) {
  glUniform4fv(shader_get_uniform_location(shader, name), 1, glm::value_ptr(*value));
}

void shader_set_mat2(Shader *shader, const char *name, glm::mat2 *mat) {
  glUniformMatrix2fv(shader_get_uniform_location(shader, name), 1, GL_FALSE, glm::value_ptr(*mat));
}

void shader_set_mat3(Shader *shader, const char *name, glm::mat3 *mat) {
  glUniformMatrix3fv(shader_get_uniform_location(shader, name), 1, GL_FALSE, glm::value_ptr(*mat));
}

void shader_set_mat4(Shader *shader, const char *name, glm::mat4 *mat) {
  glUniformMatrix4fv(shader_get_uniform_location(shader, name), 1, GL_FALSE, glm::value_ptr(*mat));
}
