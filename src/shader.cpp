void ShaderAsset::assert_shader_status_ok(uint32 new_shader, const char *path) {
  int32 status;
  glGetShaderiv(new_shader, GL_COMPILE_STATUS, &status);

  if (status != 1) {
    char message[512];
    glGetShaderInfoLog(new_shader, 512, NULL, message);
    log_info("Compiling shader %s: (status %d) (message %s)", path, status, message);
    log_error("Shader compilation failed");
    exit(EXIT_FAILURE);
  }
}


void ShaderAsset::assert_program_status_ok(uint32 new_program) {
  int32 status;
  glGetProgramiv(new_program, GL_LINK_STATUS, &status);

  if (status != 1) {
    char message[512];
    glGetProgramInfoLog(new_program, 512, NULL, message);
    log_info("Compiling program %d: (status %d) (message %s)", new_program, status, message);
    log_error("Program loading failed");
    exit(EXIT_FAILURE);
  }
}


uint32 ShaderAsset::make_shader(const char *path, const char *source, GLenum shader_type) {
  uint32 shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  assert_shader_status_ok(shader, path);
  return shader;
}


uint32 ShaderAsset::make_program(uint32 vertex_shader, uint32 fragment_shader) {
  uint32 new_program = glCreateProgram();
  glAttachShader(new_program, vertex_shader);
  glAttachShader(new_program, fragment_shader);
  glLinkProgram(new_program);
  assert_program_status_ok(new_program);
  return new_program;
}


uint32 ShaderAsset::make_program(
  uint32 vertex_shader, uint32 fragment_shader, uint32 geometry_shader
) {
  uint32 new_program = glCreateProgram();
  glAttachShader(new_program, vertex_shader);
  glAttachShader(new_program, fragment_shader);
  glAttachShader(new_program, geometry_shader);
  glLinkProgram(new_program);
  assert_program_status_ok(new_program);
  return new_program;
}


const char* ShaderAsset::load_file(Memory *memory, const char *path) {
  // TODO: It's a bit wasteful to add `common.glsl` to every single
  // shader. Maybe it would be better to somehow do it selectively.
  return ResourceManager::load_two_files(
    &memory->temp_memory_pool, SHADER_COMMON_PATH, path
  );
}


uint32 ShaderAsset::add_texture_unit(
  uint32 texture_unit, GLenum texture_unit_type
) {
  uint32 idx = ++n_texture_units;
  texture_units[idx] = texture_unit;
  texture_unit_types[idx] = texture_unit_type;
  return idx;
}


void ShaderAsset::load_uniforms() {
  did_set_texture_uniforms = false;

  // TODO: Is there a better way to do this?
  for (uint16 idx = 0; idx < MAX_N_UNIFORMS; idx++) {
    intrinsic_uniform_locations[idx] = -1;
  }

  // Bind uniform block
  uint32 uniform_block_index = glGetUniformBlockIndex(program, "shader_common");
  glUniformBlockBinding(program, uniform_block_index, 0);

  // Load uniforms
  // NOTE: We may want to skip all this stuff, because fallback on loading the locations
  // in `shader::set_*` anyway. But, it's kind of cool to know we're loading everything we can
  // up front in this function.
  uint32 new_n_intrinsic_uniforms = 0;
  GLint n_active_uniforms;
  char uniform_name[MAX_UNIFORM_NAME_LENGTH];
  GLint uniform_name_length;
  GLint uniform_size;
  GLenum uniform_type;

  glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &n_active_uniforms);

  for (GLint idx = 0; idx < n_active_uniforms; idx++) {
    glGetActiveUniform(
      program, (GLuint)idx, MAX_UNIFORM_NAME_LENGTH,
      &uniform_name_length, &uniform_size, &uniform_type, uniform_name
    );
    GLint location = glGetUniformLocation(program, uniform_name);
    if (location != -1) {
      intrinsic_uniform_locations[new_n_intrinsic_uniforms] = location;
      strcpy(intrinsic_uniform_names[new_n_intrinsic_uniforms], uniform_name);
      new_n_intrinsic_uniforms++;
    }
  }

  n_intrinsic_uniforms = new_n_intrinsic_uniforms;
}


ShaderAsset::ShaderAsset(
  Memory *memory, const char *new_name, ShaderType new_type,
  const char *vert_path, const char* frag_path
) {
  name = new_name;
  type = new_type;
  n_texture_units = 0;
  memset(texture_units, 0, sizeof(texture_units));
  memset(texture_unit_types, 0, sizeof(texture_unit_types));
  program = make_program(
    make_shader(vert_path, load_file(memory, vert_path), GL_VERTEX_SHADER),
    make_shader(frag_path, load_file(memory, frag_path), GL_FRAGMENT_SHADER)
  );
  load_uniforms();
  memory_reset_pool(&memory->temp_memory_pool);
}


ShaderAsset::ShaderAsset(
  Memory *memory, const char *new_name, ShaderType new_type,
  const char *vert_path, const char *frag_path, const char *geom_path
) {
  name = new_name;
  type = new_type;
  n_texture_units = 0;
  memset(texture_units, 0, sizeof(texture_units));
  memset(texture_unit_types, 0, sizeof(texture_unit_types));
  program = make_program(
    make_shader(vert_path, load_file(memory, vert_path), GL_VERTEX_SHADER),
    make_shader(frag_path, load_file(memory, frag_path), GL_FRAGMENT_SHADER),
    make_shader(geom_path, load_file(memory, geom_path), GL_GEOMETRY_SHADER)
  );
  load_uniforms();
  memory_reset_pool(&memory->temp_memory_pool);
}


int32 ShaderAsset::get_uniform_location(const char *uniform_name) {
  int32 uniform_idx = -1;
  for (uint32 idx = 0; idx < n_intrinsic_uniforms; idx++) {
    if (strcmp(intrinsic_uniform_names[idx], uniform_name) == 0) {
      uniform_idx = idx;
      break;
    }
  }
  if (uniform_idx == -1) {
    log_error("Had to look up location for uniform: %s", uniform_name);
    GLint location = glGetUniformLocation(program, uniform_name);
    if (location != -1) {
      uniform_idx = n_intrinsic_uniforms;
      intrinsic_uniform_locations[uniform_idx] = location;
      strcpy(intrinsic_uniform_names[uniform_idx], uniform_name);
      n_intrinsic_uniforms++;
    } else {
      if (IS_NOT_FINDING_UNIFORM_FATAL) {
        log_fatal("Could not get uniform: %s", uniform_name);
      } else {
        log_error("Could not get uniform: %s", uniform_name);
      }
    }
  }
  return intrinsic_uniform_locations[uniform_idx];
}


void ShaderAsset::set_int(const char *uniform_name, uint32 value) {
  glUniform1i(get_uniform_location(uniform_name), value);
}


void ShaderAsset::set_bool(const char *uniform_name, bool value) {
  set_int(uniform_name, (uint32)value);
}


void ShaderAsset::set_float(const char *uniform_name, float value) {
  glUniform1f(get_uniform_location(uniform_name), value);
}


void ShaderAsset::set_vec2(const char *uniform_name, glm::vec2 *value) {
  glUniform2fv(get_uniform_location(uniform_name), 1, glm::value_ptr(*value));
}


void ShaderAsset::set_vec3(const char *uniform_name, glm::vec3 *value) {
  glUniform3fv(get_uniform_location(uniform_name), 1, glm::value_ptr(*value));
}


void ShaderAsset::set_vec4(const char *uniform_name, glm::vec4 *value) {
  glUniform4fv(get_uniform_location(uniform_name), 1, glm::value_ptr(*value));
}


void ShaderAsset::set_mat2(const char *uniform_name, glm::mat2 *mat) {
  glUniformMatrix2fv(get_uniform_location(uniform_name), 1, GL_FALSE, glm::value_ptr(*mat));
}


void ShaderAsset::set_mat3(const char *uniform_name, glm::mat3 *mat) {
  glUniformMatrix3fv(get_uniform_location(uniform_name), 1, GL_FALSE, glm::value_ptr(*mat));
}


void ShaderAsset::set_mat4(const char *uniform_name, glm::mat4 *mat) {
  glUniformMatrix4fv(get_uniform_location(uniform_name), 1, GL_FALSE, glm::value_ptr(*mat));
}


ShaderAsset* ShaderAsset::get_by_name(
  Array<ShaderAsset> *assets, const char *name
) {
  for (uint32 idx = 0; idx < assets->get_size(); idx++) {
    ShaderAsset *asset = assets->get(idx);
    if (strcmp(asset->name, name) == 0) {
      return asset;
    }
  }
  log_warning("Could not find ShaderAsset with name %s", name);
  return nullptr;
}
