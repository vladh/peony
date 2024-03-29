// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "logs.hpp"
#include "glutil.hpp"


char const *
glutil::stringify_glenum(GLenum thing)
{
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


GLenum
glutil::get_texture_format_from_n_components(i32 n_components)
{
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


void
glutil::print_texture_internalformat_info(GLenum internal_format)
{
    if (!GLAD_GL_ARB_internalformat_query) {
        logs::warning("Not printing texture_internalformat as this feature is not supported on this system.");
        return;
    }

    if (!GLAD_GL_ARB_internalformat_query2) {
        logs::warning("Printing texture_internalformat, but some information may be missing, as internalformat_query2 is not supported on this system.");
    }

    GLint preferred_format;
    GLint optimal_image_format;
    GLint optimal_image_type;

    glGetInternalformativ(GL_TEXTURE_2D, internal_format, GL_INTERNALFORMAT_PREFERRED, 1, &preferred_format);
    glGetInternalformativ(GL_TEXTURE_2D, internal_format, GL_TEXTURE_IMAGE_FORMAT, 1, &optimal_image_format);
    glGetInternalformativ(GL_TEXTURE_2D, internal_format, GL_TEXTURE_IMAGE_TYPE, 1, &optimal_image_type);

    logs::info("internal format: %s", stringify_glenum(internal_format));
    logs::info("preferred format: %s", stringify_glenum(preferred_format));
    logs::info("optimal image format: %s", stringify_glenum(optimal_image_format));
    logs::info("optimal image type: %s", stringify_glenum(optimal_image_type));
    logs::print_newline();
}


void APIENTRY
glutil::debug_message_callback(
    GLenum source,
    GLenum type,
    unsigned int id,
    GLenum severity,
    GLsizei length,
    char const *message,
    const void *userParam
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
