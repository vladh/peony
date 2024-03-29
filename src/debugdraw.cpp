// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "../src_external/glad/glad.h"
#include "shaders.hpp"
#include "util.hpp"
#include "logs.hpp"
#include "debugdraw.hpp"
#include "intrinsics.hpp"


debugdraw::State *debugdraw::state = nullptr;


void
debugdraw::draw_line(v3 start_pos, v3 end_pos, v4 color)
{
    DebugDrawVertex vertices[2];
    vertices[0] = { start_pos, color };
    vertices[1] = { end_pos, color };
    push_vertices(vertices, 2);
}


void
debugdraw::draw_ray(spatial::Ray *ray, f32 length, v4 color)
{
    v3 end_pos = ray->origin + ray->direction * length;
    v3 x_axis = util::get_orthogonal_vector(&ray->direction);
    v3 z_axis = cross(ray->direction, x_axis);
    f32 chevron_size = 0.2f;
    v3 chevron_1_pos = end_pos + ((-ray->direction + x_axis) * chevron_size);
    v3 chevron_2_pos = end_pos + ((-ray->direction - x_axis) * chevron_size);
    v3 chevron_3_pos = end_pos + ((-ray->direction + z_axis) * chevron_size);
    v3 chevron_4_pos = end_pos + ((-ray->direction - z_axis) * chevron_size);
    draw_line(ray->origin, end_pos, color);
    draw_line(chevron_1_pos, end_pos, color);
    draw_line(chevron_2_pos, end_pos, color);
    draw_line(chevron_3_pos, end_pos, color);
    draw_line(chevron_4_pos, end_pos, color);
}


void
debugdraw::draw_quad(
    v3 p1, // clockwise: top left
    v3 p2, // top right
    v3 p3, // bottom right
    v3 p4, // bottom left
    v4 color
) {
    draw_line(p1, p2, color);
    draw_line(p2, p3, color);
    draw_line(p3, p4, color);
    draw_line(p4, p1, color);
}


void
debugdraw::draw_box(
    v3 p1, // clockwise top face: top left
    v3 p2, // top right
    v3 p3, // bottom right
    v3 p4, // top left
    v3 p5, // clockwise bottom face: top left
    v3 p6, // top right
    v3 p7, // bottom right
    v3 p8, // top left
    v4 color
) {
    draw_quad(p1, p2, p3, p4, color);
    draw_quad(p5, p6, p7, p8, color);
    draw_quad(p1, p2, p6, p5, color);
    draw_quad(p2, p3, p7, p6, color);
    draw_quad(p3, p4, p8, p7, color);
    draw_quad(p4, p1, p5, p8, color);
}


void
debugdraw::draw_obb(spatial::Obb *obb, v4 color)
{
    v3 z_axis = cross(obb->x_axis, obb->y_axis);
    v3 dir1 = obb->x_axis * obb->extents[0];
    v3 dir2 = obb->y_axis * obb->extents[1];
    v3 dir3 = z_axis * obb->extents[2];
    v3 p1 = obb->center - dir1 + dir2 - dir3;
    v3 p2 = obb->center + dir1 + dir2 - dir3;
    v3 p3 = obb->center + dir1 + dir2 + dir3;
    v3 p4 = obb->center - dir1 + dir2 + dir3;
    v3 p5 = obb->center - dir1 - dir2 - dir3;
    v3 p6 = obb->center + dir1 - dir2 - dir3;
    v3 p7 = obb->center + dir1 - dir2 + dir3;
    v3 p8 = obb->center - dir1 - dir2 + dir3;
    draw_quad(p1, p2, p3, p4, color);
    draw_quad(p5, p6, p7, p8, color);
    draw_quad(p1, p2, p6, p5, color);
    draw_quad(p2, p3, p7, p6, color);
    draw_quad(p3, p4, p8, p7, color);
    draw_quad(p4, p1, p5, p8, color);
}


void
debugdraw::draw_point(v3 position, f32 size, v4 color)
{
    spatial::Obb obb = {
        .center=position,
        .x_axis=v3(1.0f, 0.0f, 0.0f),
        .y_axis=v3(0.0f, 1.0f, 0.0f),
        .extents=v3(size),
    };
    draw_obb(&obb, color);
}


void
debugdraw::clear()
{
    debugdraw::state->n_vertices_pushed = 0;
}


void
debugdraw::render()
{
    glBindVertexArray(debugdraw::state->vao);
    glBindBuffer(GL_ARRAY_BUFFER, debugdraw::state->vbo);
    glBufferData(GL_ARRAY_BUFFER,
        VERTEX_SIZE * debugdraw::state->n_vertices_pushed,
        debugdraw::state->vertices, GL_STATIC_DRAW);

    glUseProgram(debugdraw::state->shader_asset.program);

    glDrawArrays(GL_LINES, 0, debugdraw::state->n_vertices_pushed);
}


void
debugdraw::init(debugdraw::State *debug_draw_state, memory::Pool *memory_pool)
{
    debugdraw::state = debug_draw_state;

    memory::Pool temp_memory_pool = {};

    // Shaders
    {
        shaders::init_shader_asset(&debugdraw::state->shader_asset,
            &temp_memory_pool, "debugdraw", shaders::Type::standard,
            "debugdraw.vert", "debugdraw.frag", "");
        debugdraw::state->shader_asset.did_set_texture_uniforms = true;
    }

    // VAO
    {
        glGenVertexArrays(1, &debugdraw::state->vao);
        glGenBuffers(1, &debugdraw::state->vbo);
        glBindVertexArray(debugdraw::state->vao);
        glBindBuffer(GL_ARRAY_BUFFER, debugdraw::state->vbo);
        glBufferData(GL_ARRAY_BUFFER, VERTEX_SIZE * MAX_N_VERTICES, NULL, GL_DYNAMIC_DRAW);

        u32 location;

        // position (vec3)
        location = 0;
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, VERTEX_SIZE,
            (void*)(0));

        // color (vec4)
        location = 1;
        glEnableVertexAttribArray(location);
        glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, VERTEX_SIZE,
            (void*)(3 * sizeof(f32)));
    }

    memory::destroy_memory_pool(&temp_memory_pool);
}


void
debugdraw::push_vertices(DebugDrawVertex vertices[], u32 n_vertices)
{
    if (debugdraw::state->n_vertices_pushed + n_vertices > MAX_N_VERTICES) {
        logs::error("Pushed too many DebugDraw vertices, did you forget to call debugdraw::clear()?");
        return;
    }
    range (0, n_vertices) {
        debugdraw::state->vertices[debugdraw::state->n_vertices_pushed + idx] = vertices[idx];
    }
    debugdraw::state->n_vertices_pushed += n_vertices;
}
