/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#include "logs.hpp"
#include "util.hpp"


const char* util::stringify_glenum(GLenum thing) {
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
      logs::warning("Unknown GLenum %d", thing);
      return "Unknown GLenum";
  }
}


GLenum util::get_texture_format_from_n_components(int32 n_components) {
  if (n_components == 1) {
    return GL_RED;
  } else if (n_components == 3) {
    return GL_BGR;
  } else if (n_components == 4) {
    return GL_BGRA;
  } else {
    logs::fatal("Don't know what to do with n_components = %d", n_components);
    return 0;
  }
}


real64 util::random(real64 min, real64 max) {
  uint32 r = rand();
  real64 r_normalized = (real64)r / (real64)RAND_MAX;
  return min + ((r_normalized) * (max - min));
}


v3 util::aiVector3D_to_glm(aiVector3D *vec) {
  return v3(vec->x, vec->y, vec->z);
}


quat util::aiQuaternion_to_glm(aiQuaternion *rotation) {
  return quat(rotation->w, rotation->x, rotation->y, rotation->z);
}


m4 util::aimatrix4x4_to_glm(aiMatrix4x4 *from) {
  m4 to;

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


void util::print_texture_internalformat_info(GLenum internal_format) {
  if (!GLAD_GL_ARB_internalformat_query) {
    logs::warning(
      "Not printing texture_internalformat as this feature is not supported "
      "on this system."
    );
    return;
  }

  if (!GLAD_GL_ARB_internalformat_query2) {
    logs::warning(
      "Printing texture_internalformat, but some information may be missing, "
      "as internalformat_query2 is not supported on this system."
    );
  }

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

  logs::info("internal format: %s", stringify_glenum(internal_format));
  logs::info("preferred format: %s", stringify_glenum(preferred_format));
  logs::info("optimal image format: %s", stringify_glenum(optimal_image_format));
  logs::info("optimal image type: %s", stringify_glenum(optimal_image_type));
  logs::print_newline();
}


void APIENTRY util::debug_message_callback(
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

  logs::warning("Debug message (%d): %s", id, message);

  switch (source) {
    case GL_DEBUG_SOURCE_API:
      logs::warning("Source: API");
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      logs::warning("Source: Window System");
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      logs::warning("Source: Shader Compiler");
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      logs::warning("Source: Third Party");
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      logs::warning("Source: Application");
      break;
    case GL_DEBUG_SOURCE_OTHER:
      logs::warning("Source: Other");
      break;
  }

  switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      logs::warning("Type: Error");
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      logs::warning("Type: Deprecated Behaviour");
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      logs::warning("Type: Undefined Behaviour");
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      logs::warning("Type: Portability");
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      logs::warning("Type: Performance");
      break;
    case GL_DEBUG_TYPE_MARKER:
      logs::warning("Type: Marker");
      break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
      logs::warning("Type: Push Group");
      break;
    case GL_DEBUG_TYPE_POP_GROUP:
      logs::warning("Type: Pop Group");
      break;
    case GL_DEBUG_TYPE_OTHER:
      logs::warning("Type: Other");
      break;
  }

  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      logs::warning("Severity: high");
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      logs::warning("Severity: medium");
      break;
    case GL_DEBUG_SEVERITY_LOW:
      logs::warning("Severity: low");
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      logs::warning("Severity: notification");
      break;
  }

  logs::print_newline();
}


real32 util::round_to_nearest_multiple(real32 n, real32 multiple_of) {
  return (floor((n) / multiple_of) * multiple_of) + multiple_of;
}


real64 util::get_us_from_duration(chrono::duration<real64> duration) {
  return chrono::duration_cast<chrono::duration<real64>>(duration).count();
}


v3 util::get_orthogonal_vector(v3 *v) {
  if (v->z < v->x) {
    return v3(v->y, -v->x, 0.0f);
  } else {
    return v3(0.0f, -v->z, v->y);
  }
}


uint32 util::kb_to_b(uint32 value) { return value * 1024; }
uint32 util::mb_to_b(uint32 value) { return kb_to_b(value) * 1024; }
uint32 util::gb_to_b(uint32 value) { return mb_to_b(value) * 1024; }
uint32 util::tb_to_b(uint32 value) { return gb_to_b(value) * 1024; }
real32 util::b_to_kb(uint32 value) { return value / 1024.0f; }
real32 util::b_to_mb(uint32 value) { return b_to_kb(value) / 1024.0f; }
real32 util::b_to_gb(uint32 value) { return b_to_mb(value) / 1024.0f; }
real32 util::b_to_tb(uint32 value) { return b_to_gb(value) / 1024.0f; }
