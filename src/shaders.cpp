// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "../src_external/pstr.h"
#include "memory.hpp"
#include "logs.hpp"
#include "files.hpp"
#include "shaders.hpp"
#include "intrinsics.hpp"


const char*
shaders::shader_type_to_string(shaders::Type shader_type)
{
    if (shader_type == shaders::Type::none) {
        return "none";
    } else if (shader_type == shaders::Type::standard) {
        return "standard";
    } else if (shader_type == shaders::Type::depth) {
        return "depth";
    } else {
        logs::error("Could not get string for shaders::Type: %d", shader_type);
        return "<unknown>";
    }
}


shaders::Type
shaders::shader_type_from_string(const char* str)
{
    if (strcmp(str, "none") == 0) {
        return shaders::Type::none;
    } else if (strcmp(str, "standard") == 0) {
        return shaders::Type::standard;
    } else if (strcmp(str, "depth") == 0) {
        return shaders::Type::depth;
    } else {
        logs::fatal("Could not parse shaders::Type: %s", str);
        return shaders::Type::none;
    }
}


bool
shaders::is_shader_asset_valid(shaders::Asset *shader_asset)
{
    return shader_asset->program > 0;
}


void
shaders::set_int(
    shaders::Asset *shader_asset, const char *uniform_name, u32 value
) {
    i32 location = get_uniform_location(shader_asset, uniform_name);
    if (location >= 0) {
        glUniform1i(location, value);
    }
}


void
shaders::set_bool(
    shaders::Asset *shader_asset, const char *uniform_name, bool value
) {
    set_int(shader_asset, uniform_name, (u32)value);
}


void
shaders::set_float(
    shaders::Asset *shader_asset, const char *uniform_name, float value
) {
    i32 location = get_uniform_location(shader_asset, uniform_name);
    if (location >= 0) {
        glUniform1f(location, value);
    }
}


void
shaders::set_vec2(shaders::Asset *shader_asset, const char *uniform_name, v2 *value)
{
    i32 location = get_uniform_location(shader_asset, uniform_name);
    if (location >= 0) {
        glUniform2fv(location, 1, glm::value_ptr(*value));
    }
}


void
shaders::set_vec3(shaders::Asset *shader_asset, const char *uniform_name, v3 *value)
{
    i32 location = get_uniform_location(shader_asset, uniform_name);
    if (location >= 0) {
        glUniform3fv(location, 1, glm::value_ptr(*value));
    }
}


void
shaders::set_vec4(shaders::Asset *shader_asset, const char *uniform_name, v4 *value)
{
    i32 location = get_uniform_location(shader_asset, uniform_name);
    if (location >= 0) {
        glUniform4fv(location, 1, glm::value_ptr(*value));
    }
}


void
shaders::set_mat2(shaders::Asset *shader_asset, const char *uniform_name, m2 *mat)
{
    i32 location = get_uniform_location(shader_asset, uniform_name);
    if (location >= 0) {
        glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(*mat));
    }
}


void
shaders::set_mat3(shaders::Asset *shader_asset, const char *uniform_name, m3 *mat)
{
    i32 location = get_uniform_location(shader_asset, uniform_name);
    if (location >= 0) {
        glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(*mat));
    }
}


void
shaders::set_mat4_multiple(
    shaders::Asset *shader_asset, u32 n, const char *uniform_name, m4 *mat
) {
    i32 location = get_uniform_location(shader_asset, uniform_name);
    if (location >= 0) {
        glUniformMatrix4fv(location, n, GL_FALSE, glm::value_ptr(*mat));
    }
}


void
shaders::set_mat4(shaders::Asset *shader_asset, const char *uniform_name, m4 *mat)
{
    i32 location = get_uniform_location(shader_asset, uniform_name);
    if (location >= 0) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(*mat));
    }
}


void
shaders::reset_texture_units(shaders::Asset *shader_asset)
{
    shader_asset->n_texture_units = 0;
}


u32
shaders::add_texture_unit(
    shaders::Asset *shader_asset,
    u32 new_texture_unit,
    GLenum new_texture_unit_type
) {
    u32 idx = ++shader_asset->n_texture_units;
    shader_asset->texture_units[idx] = new_texture_unit;
    shader_asset->texture_unit_types[idx] = new_texture_unit_type;
    return idx;
}


void
shaders::load_shader_asset(shaders::Asset *shader_asset, memory::Pool *memory_pool)
{
    shader_asset->did_set_texture_uniforms = false;

    if (!pstr_is_empty(shader_asset->geom_path)) {
        shader_asset->program = make_program(
            make_shader(shader_asset->vert_path, load_file(memory_pool, shader_asset->vert_path),
                GL_VERTEX_SHADER),
            make_shader(shader_asset->frag_path, load_frag_file(memory_pool, shader_asset->frag_path),
                GL_FRAGMENT_SHADER),
            make_shader(shader_asset->geom_path, load_file(memory_pool, shader_asset->geom_path),
                GL_GEOMETRY_SHADER));
    } else {
        shader_asset->program = make_program(
            make_shader(shader_asset->vert_path, load_file(memory_pool, shader_asset->vert_path),
                GL_VERTEX_SHADER),
            make_shader(shader_asset->frag_path, load_frag_file(memory_pool, shader_asset->frag_path),
                GL_FRAGMENT_SHADER));
    }

    load_uniforms(shader_asset);
}


shaders::Asset*
shaders::init_shader_asset(
    shaders::Asset *shader_asset,
    memory::Pool *memory_pool,
    const char *new_name, shaders::Type new_type,
    const char *vert_path, const char *frag_path, const char *geom_path
) {
    *shader_asset = {};
    pstr_copy(shader_asset->name, MAX_DEBUG_NAME_LENGTH, new_name);
    shader_asset->type = new_type;
    shader_asset->n_texture_units = 0;
    shader_asset->did_set_texture_uniforms = false;
    memset(shader_asset->texture_units, 0, sizeof(shader_asset->texture_units));
    memset(shader_asset->texture_unit_types, 0, sizeof(shader_asset->texture_unit_types));
    if (!vert_path) {
        logs::warning("Loading shader asset with no vertex shader");
    }
    if (!frag_path) {
        logs::warning("Loading shader asset with no fragment shader");
    }
    if (vert_path) {
        pstr_copy(shader_asset->vert_path, MAX_PATH, vert_path);
    }
    if (frag_path) {
        pstr_copy(shader_asset->frag_path, MAX_PATH, frag_path);
    }
    if (geom_path) {
        pstr_copy(shader_asset->geom_path, MAX_PATH, geom_path);
    }
    load_shader_asset(shader_asset, memory_pool);
    return shader_asset;
}


void
shaders::destroy_shader_asset(shaders::Asset *shader_asset)
{
    glDeleteProgram(shader_asset->program);
}


void
shaders::assert_shader_status_ok(u32 new_shader, const char *path)
{
    i32 status;
    glGetShaderiv(new_shader, GL_COMPILE_STATUS, &status);

    if (status != 1) {
        char message[MAX_GENEROUS_STRING_LENGTH];
        glGetShaderInfoLog(new_shader, MAX_GENEROUS_STRING_LENGTH, NULL, message);
        logs::info("Compiling shader %s: (status %d) (message %s)", path, status, message);
        logs::error("Shader compilation failed");
        exit(EXIT_FAILURE);
    }
}


void
shaders::assert_program_status_ok(u32 new_program)
{
    i32 status;
    glGetProgramiv(new_program, GL_LINK_STATUS, &status);

    if (status != 1) {
        char message[MAX_GENEROUS_STRING_LENGTH];
        glGetProgramInfoLog(new_program, MAX_GENEROUS_STRING_LENGTH, NULL, message);
        logs::info("Compiling program %d: (status %d) (message %s)", new_program, status, message);
        logs::error("Program loading failed");
        exit(EXIT_FAILURE);
    }
}


u32
shaders::make_shader(const char *path, const char *source, GLenum shader_type)
{
    u32 shader = glCreateShader(shader_type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    assert_shader_status_ok(shader, path);
    return shader;
}


u32
shaders::make_program(u32 vertex_shader, u32 fragment_shader)
{
    u32 new_program = glCreateProgram();
    glAttachShader(new_program, vertex_shader);
    glAttachShader(new_program, fragment_shader);
    glLinkProgram(new_program);
    glDetachShader(new_program, vertex_shader);
    glDetachShader(new_program, fragment_shader);
    assert_program_status_ok(new_program);
    return new_program;
}


u32
shaders::make_program(
    u32 vertex_shader, u32 fragment_shader, u32 geometry_shader
) {
    u32 new_program = glCreateProgram();
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


char const *
shaders::load_file(memory::Pool *memory_pool, const char *path)
{
    char full_path[MAX_PATH];
    strcpy(full_path, SHADER_DIR); // TODO: Fix unsafe strings?
    strcat(full_path, path);
    u32 f1_size = files::get_file_size(SHADER_COMMON_PATH);
    u32 f2_size = files::get_file_size(full_path);
    char *file_memory = (char*)memory::push(memory_pool, f1_size + f2_size + 1, full_path);
    files::load_file(file_memory, SHADER_COMMON_PATH);
    files::load_file(file_memory + f1_size, full_path);
    return file_memory;
}


char const *
shaders::load_frag_file(memory::Pool *memory_pool, const char *path)
{
    char full_path[MAX_PATH];
    strcpy(full_path, SHADER_DIR); // TODO: Fix unsafe strings?
    strcat(full_path, path);
    u32 f1_size = files::get_file_size(SHADER_COMMON_PATH);
    u32 f2_size = files::get_file_size(SHADER_COMMON_FRAGMENT_PATH);
    u32 f3_size = files::get_file_size(full_path);
    char *file_memory = (char*)memory::push(memory_pool, f1_size + f2_size + f3_size + 1, full_path);
    files::load_file(file_memory, SHADER_COMMON_PATH);
    files::load_file(file_memory + f1_size, SHADER_COMMON_FRAGMENT_PATH);
    files::load_file(file_memory + f1_size + f2_size, full_path);
    return file_memory;
}


i32
shaders::get_uniform_location(
    shaders::Asset *shader_asset,
    const char *uniform_name
) {
    // Look up in cache.
    for (u32 idx = 0; idx < shader_asset->n_intrinsic_uniforms; idx++) {
        if (strcmp(shader_asset->intrinsic_uniform_names[idx], uniform_name) == 0) {
            return shader_asset->intrinsic_uniform_locations[idx];
        }
    }

    // Missed cache, look up with `glGetUniformLocation()`.
    GLint location = glGetUniformLocation(shader_asset->program, uniform_name);

    if (location == -1) {
        logs::error("Could not get uniform: %s", uniform_name);
    } else {
        logs::info("Missed uniform cache for %s", uniform_name);
    }

    return location;
}


void
shaders::load_uniforms(shaders::Asset *shader_asset)
{
    for (u16 idx = 0; idx < MAX_N_UNIFORMS; idx++) {
        shader_asset->intrinsic_uniform_locations[idx] = -1;
    }

    // Bind uniform block
    u32 uniform_block_index = glGetUniformBlockIndex(shader_asset->program, "shader_common");
    glUniformBlockBinding(shader_asset->program, uniform_block_index, 0);

    // Load uniforms
    // NOTE: We may want to skip all shader_asset stuff, because fallback on loading the locations
    // in `shader::set_*` anyway. But, it's kind of cool to know we're loading everything we can
    // up front in shader_asset function.
    u32 new_n_intrinsic_uniforms = 0;
    GLint n_active_uniforms;
    char uniform_name[MAX_UNIFORM_NAME_LENGTH];
    GLint uniform_name_length;
    GLint uniform_size;
    GLenum uniform_type;

    glGetProgramiv(shader_asset->program, GL_ACTIVE_UNIFORMS, &n_active_uniforms);

    for (GLint idx = 0; idx < n_active_uniforms; idx++) {
        glGetActiveUniform(shader_asset->program, (GLuint)idx, MAX_UNIFORM_NAME_LENGTH,
            &uniform_name_length, &uniform_size, &uniform_type, uniform_name);
        GLint location = glGetUniformLocation(shader_asset->program, uniform_name);
        if (location != -1) {
            shader_asset->intrinsic_uniform_locations[new_n_intrinsic_uniforms] = location;
            strcpy(shader_asset->intrinsic_uniform_names[new_n_intrinsic_uniforms], uniform_name);
            new_n_intrinsic_uniforms++;
        }
    }

    shader_asset->n_intrinsic_uniforms = new_n_intrinsic_uniforms;
}
