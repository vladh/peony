const char* Shaders::shader_type_to_string(ShaderType shader_type) {
  if (shader_type == ShaderType::none) {
    return "none";
  } else if (shader_type == ShaderType::standard) {
    return "standard";
  } else if (shader_type == ShaderType::depth) {
    return "depth";
  } else {
    log_error("Could not get string for ShaderType: %d", shader_type);
    return "<unknown>";
  }
}


ShaderType Shaders::shader_type_from_string(const char* str) {
  if (strcmp(str, "none") == 0) {
    return ShaderType::none;
  } else if (strcmp(str, "standard") == 0) {
    return ShaderType::standard;
  } else if (strcmp(str, "depth") == 0) {
    return ShaderType::depth;
  } else {
    log_fatal("Could not parse ShaderType: %s", str);
    return ShaderType::none;
  }
}


void Shaders::assert_shader_status_ok(uint32 new_shader, const char *path) {
  int32 status;
  glGetShaderiv(new_shader, GL_COMPILE_STATUS, &status);

  if (status != 1) {
    char message[MAX_GENEROUS_STRING_LENGTH];
    glGetShaderInfoLog(new_shader, MAX_GENEROUS_STRING_LENGTH, NULL, message);
    log_info("Compiling shader %s: (status %d) (message %s)", path, status, message);
    log_error("Shader compilation failed");
    exit(EXIT_FAILURE);
  }
}


void Shaders::assert_program_status_ok(uint32 new_program) {
  int32 status;
  glGetProgramiv(new_program, GL_LINK_STATUS, &status);

  if (status != 1) {
    char message[MAX_GENEROUS_STRING_LENGTH];
    glGetProgramInfoLog(new_program, MAX_GENEROUS_STRING_LENGTH, NULL, message);
    log_info("Compiling program %d: (status %d) (message %s)", new_program, status, message);
    log_error("Program loading failed");
    exit(EXIT_FAILURE);
  }
}


uint32 Shaders::make_shader(const char *path, const char *source, GLenum shader_type) {
  uint32 shader = glCreateShader(shader_type);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  assert_shader_status_ok(shader, path);
  return shader;
}


uint32 Shaders::make_program(uint32 vertex_shader, uint32 fragment_shader) {
  uint32 new_program = glCreateProgram();
  glAttachShader(new_program, vertex_shader);
  glAttachShader(new_program, fragment_shader);
  glLinkProgram(new_program);
  glDetachShader(new_program, vertex_shader);
  glDetachShader(new_program, fragment_shader);
  assert_program_status_ok(new_program);
  return new_program;
}


uint32 Shaders::make_program(
  uint32 vertex_shader, uint32 fragment_shader, uint32 geometry_shader
) {
  uint32 new_program = glCreateProgram();
  glAttachShader(new_program, vertex_shader);
  glAttachShader(new_program, fragment_shader);
  glAttachShader(new_program, geometry_shader);
  glLinkProgram(new_program);
  glDetachShader(new_program, vertex_shader);
  glDetachShader(new_program, fragment_shader);
  glDetachShader(new_program, geometry_shader);
  assert_program_status_ok(new_program);
  return new_program;
}


const char* Shaders::load_file(MemoryPool *memory_pool, const char *path) {
  char full_path[MAX_PATH];
  strcpy(full_path, SHADER_DIR); // TODO: Fix unsafe strings?
  strcat(full_path, path);
  uint32 f1_size = Util::get_file_size(SHADER_COMMON_PATH);
  uint32 f2_size = Util::get_file_size(full_path);
  char *file_memory = (char*)Memory::push(
    memory_pool, f1_size + f2_size + 1, full_path
  );
  Util::load_file(file_memory, SHADER_COMMON_PATH);
  Util::load_file(file_memory + f1_size, full_path);
  return file_memory;
}


const char* Shaders::load_frag_file(MemoryPool *memory_pool, const char *path) {
  char full_path[MAX_PATH];
  strcpy(full_path, SHADER_DIR); // TODO: Fix unsafe strings?
  strcat(full_path, path);
  uint32 f1_size = Util::get_file_size(SHADER_COMMON_PATH);
  uint32 f2_size = Util::get_file_size(SHADER_COMMON_FRAGMENT_PATH);
  uint32 f3_size = Util::get_file_size(full_path);
  char *file_memory = (char*)Memory::push(
    memory_pool, f1_size + f2_size + f3_size + 1, full_path
  );
  Util::load_file(file_memory, SHADER_COMMON_PATH);
  Util::load_file(file_memory + f1_size, SHADER_COMMON_FRAGMENT_PATH);
  Util::load_file(file_memory + f1_size + f2_size, full_path);
  return file_memory;
}


int32 Shaders::get_uniform_location(
  ShaderAsset *shader_asset,
  const char *uniform_name
) {
  // Look up in cache.
  for (uint32 idx = 0; idx < shader_asset->n_intrinsic_uniforms; idx++) {
    if (strcmp(shader_asset->intrinsic_uniform_names[idx], uniform_name) == 0) {
      return shader_asset->intrinsic_uniform_locations[idx];
    }
  }

  // Missed cache, look up with `glGetUniformLocation()`.
  GLint location = glGetUniformLocation(shader_asset->program, uniform_name);

  if (location == -1) {
    log_error("Could not get uniform: %s", uniform_name);
  } else {
    log_info("Missed uniform cache for %s", uniform_name);
  }

  return location;
}


void Shaders::set_int(
  ShaderAsset *shader_asset, const char *uniform_name, uint32 value
) {
  int32 location = get_uniform_location(shader_asset, uniform_name);
  if (location >= 0) {
    glUniform1i(location, value);
  }
}


void Shaders::set_bool(
  ShaderAsset *shader_asset, const char *uniform_name, bool value
) {
  set_int(shader_asset, uniform_name, (uint32)value);
}


void Shaders::set_float(
  ShaderAsset *shader_asset, const char *uniform_name, float value
) {
  int32 location = get_uniform_location(shader_asset, uniform_name);
  if (location >= 0) {
    glUniform1f(location, value);
  }
}


void Shaders::set_vec2(
  ShaderAsset *shader_asset, const char *uniform_name, glm::vec2 *value
) {
  int32 location = get_uniform_location(shader_asset, uniform_name);
  if (location >= 0) {
    glUniform2fv(location, 1, glm::value_ptr(*value));
  }
}


void Shaders::set_vec3(
  ShaderAsset *shader_asset, const char *uniform_name, glm::vec3 *value
) {
  int32 location = get_uniform_location(shader_asset, uniform_name);
  if (location >= 0) {
    glUniform3fv(location, 1, glm::value_ptr(*value));
  }
}


void Shaders::set_vec4(
  ShaderAsset *shader_asset, const char *uniform_name, glm::vec4 *value
) {
  int32 location = get_uniform_location(shader_asset, uniform_name);
  if (location >= 0) {
    glUniform4fv(location, 1, glm::value_ptr(*value));
  }
}


void Shaders::set_mat2(
  ShaderAsset *shader_asset, const char *uniform_name, glm::mat2 *mat
) {
  int32 location = get_uniform_location(shader_asset, uniform_name);
  if (location >= 0) {
    glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(*mat));
  }
}


void Shaders::set_mat3(
  ShaderAsset *shader_asset, const char *uniform_name, glm::mat3 *mat
) {
  int32 location = get_uniform_location(shader_asset, uniform_name);
  if (location >= 0) {
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(*mat));
  }
}


void Shaders::set_mat4(
  ShaderAsset *shader_asset, const char *uniform_name, glm::mat4 *mat
) {
  int32 location = get_uniform_location(shader_asset, uniform_name);
  if (location >= 0) {
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(*mat));
  }
}


void Shaders::reset_texture_units(ShaderAsset *shader_asset) {
  shader_asset->n_texture_units = 0;
}


uint32 Shaders::add_texture_unit(
  ShaderAsset *shader_asset,
  uint32 new_texture_unit,
  GLenum new_texture_unit_type
) {
  uint32 idx = ++shader_asset->n_texture_units;
  shader_asset->texture_units[idx] = new_texture_unit;
  shader_asset->texture_unit_types[idx] = new_texture_unit_type;
  return idx;
}


void Shaders::load_uniforms(ShaderAsset *shader_asset) {
  for (uint16 idx = 0; idx < MAX_N_UNIFORMS; idx++) {
    shader_asset->intrinsic_uniform_locations[idx] = -1;
  }

  // Bind uniform block
  uint32 uniform_block_index = glGetUniformBlockIndex(
    shader_asset->program, "shader_common"
  );
  glUniformBlockBinding(shader_asset->program, uniform_block_index, 0);

  // Load uniforms
  // NOTE: We may want to skip all shader_asset stuff, because fallback on loading the locations
  // in `shader::set_*` anyway. But, it's kind of cool to know we're loading everything we can
  // up front in shader_asset function.
  uint32 new_n_intrinsic_uniforms = 0;
  GLint n_active_uniforms;
  char uniform_name[MAX_UNIFORM_NAME_LENGTH];
  GLint uniform_name_length;
  GLint uniform_size;
  GLenum uniform_type;

  glGetProgramiv(shader_asset->program, GL_ACTIVE_UNIFORMS, &n_active_uniforms);

  for (GLint idx = 0; idx < n_active_uniforms; idx++) {
    glGetActiveUniform(
      shader_asset->program, (GLuint)idx, MAX_UNIFORM_NAME_LENGTH,
      &uniform_name_length, &uniform_size, &uniform_type, uniform_name
    );
    GLint location = glGetUniformLocation(shader_asset->program, uniform_name);
    if (location != -1) {
      shader_asset->intrinsic_uniform_locations[new_n_intrinsic_uniforms] = location;
      strcpy(
        shader_asset->intrinsic_uniform_names[new_n_intrinsic_uniforms],
        uniform_name
      );
      new_n_intrinsic_uniforms++;
    }
  }

  shader_asset->n_intrinsic_uniforms = new_n_intrinsic_uniforms;
}


void Shaders::load_shader_asset(ShaderAsset *shader_asset) {
  MemoryPool temp_memory_pool = {};
  shader_asset->did_set_texture_uniforms = false;

  if (!Str::is_empty(shader_asset->geom_path)) {
    shader_asset->program = make_program(
      make_shader(
        shader_asset->vert_path,
        load_file(&temp_memory_pool, shader_asset->vert_path), GL_VERTEX_SHADER
      ),
      make_shader(
        shader_asset->frag_path,
        load_frag_file(&temp_memory_pool, shader_asset->frag_path), GL_FRAGMENT_SHADER
      ),
      make_shader(
        shader_asset->geom_path,
        load_file(&temp_memory_pool, shader_asset->geom_path), GL_GEOMETRY_SHADER
      )
    );
  } else {
    shader_asset->program = make_program(
      make_shader(
        shader_asset->vert_path,
        load_file(&temp_memory_pool, shader_asset->vert_path), GL_VERTEX_SHADER
      ),
      make_shader(
        shader_asset->frag_path,
        load_frag_file(&temp_memory_pool, shader_asset->frag_path), GL_FRAGMENT_SHADER
      )
    );
  }

  load_uniforms(shader_asset);

  Memory::destroy_memory_pool(&temp_memory_pool);
}


ShaderAsset* Shaders::init_shader_asset(
  ShaderAsset *shader_asset,
  const char *new_name, ShaderType new_type,
  const char *vert_path, const char *frag_path, const char *geom_path
) {
  strcpy(shader_asset->name, new_name);
  shader_asset->type = new_type;
  shader_asset->n_texture_units = 0;
  shader_asset->did_set_texture_uniforms = false;
  memset(shader_asset->texture_units, 0, sizeof(shader_asset->texture_units));
  memset(shader_asset->texture_unit_types, 0, sizeof(shader_asset->texture_unit_types));
  strcpy(shader_asset->vert_path, vert_path);
  strcpy(shader_asset->frag_path, frag_path);
  strcpy(shader_asset->geom_path, geom_path);
  load_shader_asset(shader_asset);
  return shader_asset;
}


void Shaders::destroy_shader_asset(ShaderAsset *shader_asset) {
  glDeleteProgram(shader_asset->program);
}
