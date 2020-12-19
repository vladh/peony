const char* shader_type_to_string(ShaderType shader_type) {
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


ShaderType shader_type_from_string(const char* str) {
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
  uint32 f1_size = Util::get_file_size(SHADER_COMMON_PATH);
  uint32 f2_size = Util::get_file_size(path);
  char *file_memory = (char*)memory->temp_memory_pool.push(
    f1_size + f2_size + 1, path
  );
  Util::load_file(file_memory, SHADER_COMMON_PATH);
  Util::load_file(file_memory + f1_size, path);
  return file_memory;
}


const char* ShaderAsset::load_frag_file(Memory *memory, const char *path) {
  uint32 f1_size = Util::get_file_size(SHADER_COMMON_PATH);
  uint32 f2_size = Util::get_file_size(SHADER_COMMON_FRAGMENT_PATH);
  uint32 f3_size = Util::get_file_size(path);
  char *file_memory = (char*)memory->temp_memory_pool.push(
    f1_size + f2_size + f3_size + 1, path
  );
  Util::load_file(file_memory, SHADER_COMMON_PATH);
  Util::load_file(file_memory + f1_size, SHADER_COMMON_FRAGMENT_PATH);
  Util::load_file(file_memory + f1_size + f2_size, path);
  return file_memory;
}


void ShaderAsset::reset_texture_units() {
  this->n_texture_units = 0;
}


uint32 ShaderAsset::add_texture_unit(
  uint32 new_texture_unit, GLenum new_texture_unit_type
) {
  uint32 idx = ++this->n_texture_units;
  this->texture_units[idx] = new_texture_unit;
  this->texture_unit_types[idx] = new_texture_unit_type;
  return idx;
}


void ShaderAsset::load_uniforms() {
  this->did_set_texture_uniforms = false;

  for (uint16 idx = 0; idx < MAX_N_UNIFORMS; idx++) {
    intrinsic_uniform_locations[idx] = -1;
  }

  // Bind uniform block
  uint32 uniform_block_index = glGetUniformBlockIndex(this->program, "shader_common");
  glUniformBlockBinding(this->program, uniform_block_index, 0);

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

  glGetProgramiv(this->program, GL_ACTIVE_UNIFORMS, &n_active_uniforms);

  for (GLint idx = 0; idx < n_active_uniforms; idx++) {
    glGetActiveUniform(
      this->program, (GLuint)idx, MAX_UNIFORM_NAME_LENGTH,
      &uniform_name_length, &uniform_size, &uniform_type, uniform_name
    );
    GLint location = glGetUniformLocation(this->program, uniform_name);
    if (location != -1) {
      intrinsic_uniform_locations[new_n_intrinsic_uniforms] = location;
      strcpy(intrinsic_uniform_names[new_n_intrinsic_uniforms], uniform_name);
      new_n_intrinsic_uniforms++;
    }
  }

  n_intrinsic_uniforms = new_n_intrinsic_uniforms;
}


void ShaderAsset::load(Memory *memory) {
  if (this->geom_path) {
    this->program = make_program(
      make_shader(this->vert_path, load_file(memory, this->vert_path), GL_VERTEX_SHADER),
      make_shader(this->frag_path, load_frag_file(memory, this->frag_path), GL_FRAGMENT_SHADER),
      make_shader(this->geom_path, load_file(memory, this->geom_path), GL_GEOMETRY_SHADER)
    );
  } else {
    this->program = make_program(
      make_shader(this->vert_path, load_file(memory, this->vert_path), GL_VERTEX_SHADER),
      make_shader(this->frag_path, load_frag_file(memory, this->frag_path), GL_FRAGMENT_SHADER)
    );
  }

  load_uniforms();
  memory->temp_memory_pool.reset();
}


ShaderAsset::ShaderAsset(
  Memory *memory, const char *new_name, ShaderType new_type,
  const char *vert_path, const char *frag_path, const char *geom_path
) {
  this->name = new_name;
  this->type = new_type;
  this->n_texture_units = 0;
  memset(this->texture_units, 0, sizeof(this->texture_units));
  memset(this->texture_unit_types, 0, sizeof(this->texture_unit_types));
  this->vert_path = vert_path;
  this->frag_path = frag_path;
  this->geom_path = geom_path;
  load(memory);
}


int32 ShaderAsset::get_uniform_location(const char *uniform_name) {
  // Look up in cache.
  for (uint32 idx = 0; idx < n_intrinsic_uniforms; idx++) {
    if (strcmp(intrinsic_uniform_names[idx], uniform_name) == 0) {
      return intrinsic_uniform_locations[idx];
    }
  }

  // Missed cache, look up with `glGetUniformLocation()`.
  GLint location = glGetUniformLocation(this->program, uniform_name);

#if 0
  uniform_idx = n_intrinsic_uniforms;
  intrinsic_uniform_locations[uniform_idx] = location;
  strcpy(intrinsic_uniform_names[uniform_idx], uniform_name);
  n_intrinsic_uniforms++;
#endif

  if (location == -1) {
    log_error("Could not get uniform: %s", uniform_name);
  } else {
    log_info("Missed uniform cache for %s", uniform_name);
  }

  return location;
}


void ShaderAsset::set_int(const char *uniform_name, uint32 value) {
  int32 location = get_uniform_location(uniform_name);
  if (location >= 0) {
    glUniform1i(location, value);
  }
}


void ShaderAsset::set_bool(const char *uniform_name, bool value) {
  set_int(uniform_name, (uint32)value);
}


void ShaderAsset::set_float(const char *uniform_name, float value) {
  int32 location = get_uniform_location(uniform_name);
  if (location >= 0) {
    glUniform1f(location, value);
  }
}


void ShaderAsset::set_vec2(const char *uniform_name, glm::vec2 *value) {
  int32 location = get_uniform_location(uniform_name);
  if (location >= 0) {
    glUniform2fv(location, 1, glm::value_ptr(*value));
  }
}


void ShaderAsset::set_vec3(const char *uniform_name, glm::vec3 *value) {
  int32 location = get_uniform_location(uniform_name);
  if (location >= 0) {
    glUniform3fv(location, 1, glm::value_ptr(*value));
  }
}


void ShaderAsset::set_vec4(const char *uniform_name, glm::vec4 *value) {
  int32 location = get_uniform_location(uniform_name);
  if (location >= 0) {
    glUniform4fv(location, 1, glm::value_ptr(*value));
  }
}


void ShaderAsset::set_mat2(const char *uniform_name, glm::mat2 *mat) {
  int32 location = get_uniform_location(uniform_name);
  if (location >= 0) {
    glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(*mat));
  }
}


void ShaderAsset::set_mat3(const char *uniform_name, glm::mat3 *mat) {
  int32 location = get_uniform_location(uniform_name);
  if (location >= 0) {
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(*mat));
  }
}


void ShaderAsset::set_mat4(const char *uniform_name, glm::mat4 *mat) {
  int32 location = get_uniform_location(uniform_name);
  if (location >= 0) {
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(*mat));
  }
}


ShaderAsset* ShaderAsset::get_by_name(
  Array<ShaderAsset> *assets, const char *name
) {
  for (uint32 idx = 0; idx < assets->size; idx++) {
    ShaderAsset *asset = assets->get(idx);
    if (strcmp(asset->name, name) == 0) {
      return asset;
    }
  }
  log_warning("Could not find ShaderAsset with name %s", name);
  return nullptr;
}
