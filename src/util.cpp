unsigned char* Util::load_image(
  const char *path, int32 *width, int32 *height, int32 *n_channels, bool should_flip
) {
  // TODO: Change this to use our custom allocator.
  stbi_set_flip_vertically_on_load(should_flip);
  unsigned char *image_data = stbi_load(
    path, width, height, n_channels, 0
  );
  return image_data;
}


unsigned char* Util::load_image(
  const char *path, int32 *width, int32 *height, int32 *n_channels
) {
  return load_image(path, width, height, n_channels, true);
}


void Util::free_image(unsigned char *image_data) {
  stbi_image_free(image_data);
}


uint32 Util::get_file_size(const char *path) {
  FILE* f = fopen(path, "rb");
  if (!f) {
    log_error("Could not open file %s.", path);
    return 0;
  }
  fseek(f, 0, SEEK_END);
  uint32 size = ftell(f);
  fclose(f);
  return size;
}


const char* Util::load_file(MemoryPool *pool, const char *path) {
  FILE* f = fopen(path, "rb");
  if (!f) {
    log_error("Could not open file %s.", path);
    return nullptr;
  }
  fseek(f, 0, SEEK_END);
  uint32 file_size = ftell(f);
  fseek(f, 0, SEEK_SET);

  char *string = (char*)pool->push(file_size + 1, path);
  size_t result = fread(string, file_size, 1, f);
  fclose(f);
  if (result != 1) {
    log_error("Could not read from file %s.", path);
    return nullptr;
  }

  string[file_size] = 0;
  return string;
}

const char* Util::load_file(char *string, const char *path) {
  FILE* f = fopen(path, "rb");
  if (!f) {
    log_error("Could not open file %s.", path);
    return nullptr;
  }
  fseek(f, 0, SEEK_END);
  uint32 file_size = ftell(f);
  fseek(f, 0, SEEK_SET);

  size_t result = fread(string, file_size, 1, f);
  fclose(f);
  if (result != 1) {
    log_error("Could not read from file %s.", path);
    return nullptr;
  }

  string[file_size] = 0;
  return string;
}

const char* Util::stringify_glenum(GLenum thing) {
  switch (thing) {
    case 0:
      return "(none)";
    case GL_BYTE:
      return "GLbyte";
    case GL_UNSIGNED_BYTE:
      return "GLubyte";
    case GL_SHORT:
      return "GLshort";
    case GL_UNSIGNED_SHORT:
      return "GLushort";
    case GL_INT:
      return "GLint";
    case GL_UNSIGNED_INT:
      return "GLuint";
    case GL_HALF_FLOAT:
      return "GLhalf";
    case GL_FLOAT:
      return "GLfloat";
    case GL_DOUBLE:
      return "GLdouble";
    case GL_RGB8:
      return "GL_RGB8";
    case GL_RGBA8:
      return "GL_RGBA8";
    case GL_SRGB8:
      return "GL_SRGB8";
    case GL_UNSIGNED_INT_8_8_8_8:
      return "GL_UNSIGNED_INT_8_8_8_8";
    case GL_UNSIGNED_INT_8_8_8_8_REV:
      return "GL_UNSIGNED_INT_8_8_8_8_REV";
    case GL_DEPTH_COMPONENT:
      return "GL_DEPTH_COMPONENT";
    case GL_DEPTH_STENCIL:
      return "GL_DEPTH_STENCIL";
    case GL_RED:
      return "GL_RED";
    case GL_RGB:
      return "GL_RGB";
    case GL_RGBA:
      return "GL_RGBA";
    default:
      log_warning("Unknown GLenum %d", thing);
      return "Unknown GLenum";
  }
}

GLenum Util::get_texture_format_from_n_components(int32 n_components) {
  if (n_components == 1) {
    return GL_RED;
  } else if (n_components == 3) {
    return GL_BGR;
  } else if (n_components == 4) {
    return GL_BGRA;
  } else {
    log_fatal("Don't know what to do with n_components = %d", n_components);
    return 0;
  }
}


real64 Util::random(real64 min, real64 max) {
  uint32 r = rand();
  real64 r_normalized = (real64)r / (real64)RAND_MAX;
  return min + ((r_normalized) * (max - min));
}


void Util::make_plane(
  MemoryPool *memory_pool,
  uint32 x_size, uint32 z_size,
  uint32 n_x_segments, uint32 n_z_segments,
  uint32 *n_vertices, uint32 *n_indices,
  real32 **vertex_data, uint32 **index_data
) {
  uint32 idx_vertices = 0;
  uint32 idx_indices = 0;
  *n_vertices = 0;
  *n_indices = 0;

  uint32 vertex_data_length = (n_x_segments + 1) * (n_z_segments + 1) * 8;
  uint32 index_data_length = (n_x_segments) * (n_z_segments) * 6;

  *vertex_data = (real32*)memory_pool->push(
    sizeof(real32) * vertex_data_length, "plane_vertex_data"
  );
  *index_data = (uint32*)memory_pool->push(
    sizeof(uint32) * index_data_length, "plane_index_data"
  );

  for (uint32 idx_x = 0; idx_x <= n_x_segments; idx_x++) {
    for (uint32 idx_z = 0; idx_z <= n_z_segments; idx_z++) {
      real32 x_segment = (real32)idx_x / (real32)n_x_segments;
      real32 z_segment = (real32)idx_z / (real32)n_z_segments;
      real32 x_pos = x_segment * x_size - (x_size / 2);
      real32 y_pos = 0;
      real32 z_pos = z_segment * z_size - (z_size / 2);

      // Position
      (*vertex_data)[idx_vertices++] = x_pos;
      (*vertex_data)[idx_vertices++] = y_pos;
      (*vertex_data)[idx_vertices++] = z_pos;
      // Normal
      (*vertex_data)[idx_vertices++] = 0.0f;
      (*vertex_data)[idx_vertices++] = 1.0f;
      (*vertex_data)[idx_vertices++] = 0.0f;
      // Tex coords
      (*vertex_data)[idx_vertices++] = x_segment;
      (*vertex_data)[idx_vertices++] = z_segment;

      (*n_vertices)++;
    }
  }

  // NOTE: Counterclockwise winding order. I could swear this code is CW
  // order though. Not sure where the confusion happens.
  for (uint32 idx_x = 0; idx_x < n_x_segments; idx_x++) {
    for (uint32 idx_z = 0; idx_z < n_z_segments; idx_z++) {
      // This current vertex.
      (*index_data)[idx_indices++] = (idx_x * (n_z_segments + 1)) + idx_z;
      // Next row, right of this one.
      (*index_data)[idx_indices++] = ((idx_x + 1) * (n_z_segments + 1)) + idx_z + 1;
      // Next row, under this one.
      (*index_data)[idx_indices++] = ((idx_x + 1) * (n_z_segments + 1)) + idx_z;

      // This current vertex.
      (*index_data)[idx_indices++] = (idx_x * (n_z_segments + 1)) + idx_z;
      // This row, right of this one.
      (*index_data)[idx_indices++] = (idx_x * (n_z_segments + 1)) + idx_z + 1;
      // Next row, right of this one.
      (*index_data)[idx_indices++] = ((idx_x + 1) * (n_z_segments + 1)) + idx_z + 1;

      (*n_indices) += 6;
    }
  }
}


void Util::make_sphere(
  MemoryPool *memory_pool,
  uint32 n_x_segments, uint32 n_y_segments,
  uint32 *n_vertices, uint32 *n_indices,
  real32 **vertex_data, uint32 **index_data
) {
  uint32 idx_vertices = 0;
  uint32 idx_indices = 0;
  *n_vertices = 0;
  *n_indices = 0;

  uint32 vertex_data_length = (n_x_segments + 1) * (n_y_segments + 1) * 8;
  uint32 index_data_length = (n_x_segments + 1) * (n_y_segments) * 2;

  *vertex_data = (real32*)memory_pool->push(
    sizeof(real32) * vertex_data_length, "sphere_vertex_data"
  );
  *index_data = (uint32*)memory_pool->push(
    sizeof(uint32) * index_data_length, "sphere_index_data"
  );

  for (uint32 y = 0; y <= n_y_segments; y++) {
    for (uint32 x = 0; x <= n_x_segments; x++) {
      real32 x_segment = (real32)x / (real32)n_x_segments;
      real32 y_segment = (real32)y / (real32)n_y_segments;
      real32 x_pos = cos(x_segment * 2.0f * PI32) * sin(y_segment * PI32);
      real32 y_pos = cos(y_segment * PI32);
      real32 z_pos = sin(x_segment * 2.0f * PI32) * sin(y_segment * PI32);

      // Position
      (*vertex_data)[idx_vertices++] = x_pos;
      (*vertex_data)[idx_vertices++] = y_pos;
      (*vertex_data)[idx_vertices++] = z_pos;
      // Normal
      (*vertex_data)[idx_vertices++] = x_pos;
      (*vertex_data)[idx_vertices++] = y_pos;
      (*vertex_data)[idx_vertices++] = z_pos;
      // Tex coords
      (*vertex_data)[idx_vertices++] = x_segment;
      (*vertex_data)[idx_vertices++] = y_segment;

      (*n_vertices)++;
    }
  }

  for (uint32 y = 0; y < n_y_segments; y++) {
    if (y % 2 == 0) {
      /* for (int32 x = n_x_segments; x >= 0; x--) { */
      for (uint32 x = 0; x <= n_x_segments; x++) {
        (*index_data)[idx_indices++] = (y + 1) * (n_x_segments + 1) + x;
        (*index_data)[idx_indices++] = y * (n_x_segments + 1) + x;
        (*n_indices) += 2;
      }
    } else {
      /* for (uint32 x = 0; x <= n_x_segments; x++) { */
      for (int32 x = n_x_segments; x >= 0; x--) {
        (*index_data)[idx_indices++] = y * (n_x_segments + 1) + x;
        (*index_data)[idx_indices++] = (y + 1) * (n_x_segments + 1) + x;
        (*n_indices) += 2;
      }
    }
  }
}


glm::mat4 Util::aimatrix4x4_to_glm(aiMatrix4x4* from) {
  glm::mat4 to;

  to[0][0] = (GLfloat)from->a1;
  to[0][1] = (GLfloat)from->b1;
  to[0][2] = (GLfloat)from->c1;
  to[0][3] = (GLfloat)from->d1;

  to[1][0] = (GLfloat)from->a2;
  to[1][1] = (GLfloat)from->b2;
  to[1][2] = (GLfloat)from->c2;
  to[1][3] = (GLfloat)from->d2;

  to[2][0] = (GLfloat)from->a3;
  to[2][1] = (GLfloat)from->b3;
  to[2][2] = (GLfloat)from->c3;
  to[2][3] = (GLfloat)from->d3;

  to[3][0] = (GLfloat)from->a4;
  to[3][1] = (GLfloat)from->b4;
  to[3][2] = (GLfloat)from->c4;
  to[3][3] = (GLfloat)from->d4;

  return to;
}


void Util::print_texture_internalformat_info(GLenum internal_format) {
  GLint preferred_format;
  GLint optimal_image_format;
  GLint optimal_image_type;

  glGetInternalformativ(
    GL_TEXTURE_2D, internal_format, GL_INTERNALFORMAT_PREFERRED, 1, &preferred_format
  );
  glGetInternalformativ(
    GL_TEXTURE_2D, internal_format, GL_TEXTURE_IMAGE_FORMAT, 1, &optimal_image_format
  );
  glGetInternalformativ(
    GL_TEXTURE_2D, internal_format, GL_TEXTURE_IMAGE_TYPE, 1, &optimal_image_type
  );

  log_info("internal format: %s", Util::stringify_glenum(internal_format));
  log_info("preferred format: %s", Util::stringify_glenum(preferred_format));
  log_info("optimal image format: %s", Util::stringify_glenum(optimal_image_format));
  log_info("optimal image type: %s", Util::stringify_glenum(optimal_image_type));
  log_newline();
}


void APIENTRY Util::debug_message_callback(
  GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
  const char *message, const void *userParam
) {
  // Ignore insignificant error/warning codes
  if (
    // Framebuffer detailed info: The driver allocated storage for
    // renderbuffer 1.
    id == 131169 ||
    // Program/shader state performance warning: Vertex shader in program 19
    // is being recompiled based on GL state.
    id == 131218 ||
    // Buffer detailed info: Buffer object 1522 (bound to
    // GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB (0), and GL_ARRAY_BUFFER_ARB,
    // usage hint is GL_DYNAMIC_DRAW) will use VIDEO memory as the source for
    // buffer object operations.
    id == 131185 ||
    // Texture state usage warning: The texture object (0) bound to texture
    // image unit 4 does not have a defined base level and cannot be used for
    // texture mapping.
    id == 131204 ||
    // Pixel-path performance warning: Pixel transfer is synchronized with 3D rendering.
    id == 131154
  ) {
    return;
  }

  log_warning("Debug message (%d): %s", id, message);

  switch (source) {
    case GL_DEBUG_SOURCE_API:
      log_warning("Source: API");
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      log_warning("Source: Window System");
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      log_warning("Source: Shader Compiler");
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      log_warning("Source: Third Party");
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      log_warning("Source: Application");
      break;
    case GL_DEBUG_SOURCE_OTHER:
      log_warning("Source: Other");
      break;
  }

  switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      log_warning("Type: Error");
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      log_warning("Type: Deprecated Behaviour");
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      log_warning("Type: Undefined Behaviour");
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      log_warning("Type: Portability");
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      log_warning("Type: Performance");
      break;
    case GL_DEBUG_TYPE_MARKER:
      log_warning("Type: Marker");
      break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
      log_warning("Type: Push Group");
      break;
    case GL_DEBUG_TYPE_POP_GROUP:
      log_warning("Type: Pop Group");
      break;
    case GL_DEBUG_TYPE_OTHER:
      log_warning("Type: Other");
      break;
  }

  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      log_warning("Severity: high");
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      log_warning("Severity: medium");
      break;
    case GL_DEBUG_SEVERITY_LOW:
      log_warning("Severity: low");
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      log_warning("Severity: notification");
      break;
  }

  log_newline();
}


real32 Util::round_to_nearest_multiple(real32 n, real32 multiple_of) {
  return (floor((n) / multiple_of) * multiple_of) + multiple_of;
}
