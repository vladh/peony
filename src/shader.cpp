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


uint32 shader_make_program(uint32 vertex_shader, uint32 fragment_shader) {
  uint32 shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  shader_assert_program_status_ok(shader_program);
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
  return shader_program;
}


const char* shader_load_file(Memory *memory, const char *path) {
  // TODO: It's a bit wasteful to add `common.glsl` to every single
  // shader. Maybe it would be better to somehow do it selectively.
  return util_load_two_files(
    &memory->temp_memory_pool, SHADER_COMMON_PATH, path
  );
}


uint32 shader_add_texture_unit(
  Shader *shader, uint32 texture_unit, GLenum texture_unit_type
) {
  uint32 idx = ++shader->n_texture_units;
  shader->texture_units[idx] = texture_unit;
  shader->texture_unit_types[idx] = texture_unit_type;
  return idx;
}


void shader_init(Shader *shader, uint32 program, ShaderType type) {
  shader->program = program;

  // TODO: Is there a better way to do this?
  for (uint16 idx = 0; idx < MAX_N_UNIFORMS; idx++) {
    shader->intrinsic_uniform_locations[idx] = -1;
  }

  shader->n_texture_units = 0;
  memset(shader->texture_units, 0, sizeof(shader->texture_units));

  shader->type = type;
  shader->did_set_texture_uniforms = false;

  // Bind uniform block
  uint32 uniform_block_index = glGetUniformBlockIndex(shader->program, "shader_common");
  glUniformBlockBinding(shader->program, uniform_block_index, 0);

  // Load uniforms
  // NOTE: We may want to skip all this stuff, because fallback on loading the locations
  // in `shader_set_*` anyway. But, it's kind of cool to know we're loading everything we can
  // up front in this function.
  uint32 n_intrinsic_uniforms = 0;
  GLint n_active_uniforms;
  char uniform_name[MAX_UNIFORM_NAME_LENGTH];
  GLint uniform_name_length;
  GLint uniform_size;
  GLenum uniform_type;

  glGetProgramiv(shader->program, GL_ACTIVE_UNIFORMS, &n_active_uniforms);

  for (GLint idx = 0; idx < n_active_uniforms; idx++) {
    glGetActiveUniform(
      shader->program, (GLuint)idx, MAX_UNIFORM_NAME_LENGTH,
      &uniform_name_length, &uniform_size, &uniform_type, uniform_name
    );
    GLint location = glGetUniformLocation(shader->program, uniform_name);
    if (location != -1) {
      shader->intrinsic_uniform_locations[n_intrinsic_uniforms] = location;
      strcpy(shader->intrinsic_uniform_names[n_intrinsic_uniforms], uniform_name);
      n_intrinsic_uniforms++;
    }
  }

  shader->n_intrinsic_uniforms = n_intrinsic_uniforms;
}


void shader_init(
  Shader *shader, Memory *memory, ShaderType type,
  const char* vert_path, const char *frag_path, const char *geom_path
) {
  uint32 program = shader_make_program(
    shader_load(vert_path, shader_load_file(memory, vert_path), GL_VERTEX_SHADER),
    shader_load(frag_path, shader_load_file(memory, frag_path), GL_FRAGMENT_SHADER),
    shader_load(geom_path, shader_load_file(memory, geom_path), GL_GEOMETRY_SHADER)
  );

  shader_init(shader, program, type);
}


void shader_init(
  Shader *shader, Memory *memory, ShaderType type,
  const char* vert_path, const char *frag_path
) {
  uint32 program = shader_make_program(
    shader_load(vert_path, shader_load_file(memory, vert_path), GL_VERTEX_SHADER),
    shader_load(frag_path, shader_load_file(memory, frag_path), GL_FRAGMENT_SHADER)
  );

  shader_init(shader, program, type);
}


ShaderAsset* shader_make_asset(
  ShaderAsset *asset, Memory *memory, const char *name, ShaderType type,
  const char *vert_path, const char* frag_path
) {
  asset->info.name = name;
  shader_init(&asset->shader, memory, type, vert_path, frag_path);
  memory_reset_pool(&memory->temp_memory_pool);
  return asset;
}


ShaderAsset* shader_make_asset(
  ShaderAsset *asset, Memory *memory, const char *name, ShaderType type,
  const char *vert_path, const char *frag_path, const char *geom_path
) {
  asset->info.name = name;
  shader_init(&asset->shader, memory, type, vert_path, frag_path, geom_path);
  memory_reset_pool(&memory->temp_memory_pool);
  return asset;
}


int32 shader_get_uniform_location(Shader *shader, const char *name) {
  int32 uniform_idx = -1;
  for (uint32 idx = 0; idx < shader->n_intrinsic_uniforms; idx++) {
    if (strcmp(shader->intrinsic_uniform_names[idx], name) == 0) {
      uniform_idx = idx;
      break;
    }
  }
  if (uniform_idx == -1) {
    log_error("Had to look up location for uniform: %s", name);
    GLint location = glGetUniformLocation(shader->program, name);
    if (location != -1) {
      uniform_idx = shader->n_intrinsic_uniforms;
      shader->intrinsic_uniform_locations[uniform_idx] = location;
      strcpy(shader->intrinsic_uniform_names[uniform_idx], name);
      shader->n_intrinsic_uniforms++;
    } else {
      log_fatal("Could not get uniform: %s", name);
    }
  }
  return shader->intrinsic_uniform_locations[uniform_idx];
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
