// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "geom.hpp"


void
geom::setup_mesh_vertex_buffers(
    Mesh *mesh,
    Vertex *vertex_data, u32 n_vertices,
    u32 *index_data, u32 n_indices
) {
    assert(vertex_data && n_vertices > 0);

    u32 vertex_size = sizeof(Vertex);
    u32 index_size = sizeof(u32);

    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    glBindVertexArray(mesh->vao);

    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_size * n_vertices, vertex_data, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_size * n_indices, index_data, GL_STATIC_DRAW);

    u32 location;

    location = 0;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, vertex_size, (void*)offsetof(Vertex, position));

    location = 1;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, 3, GL_FLOAT, GL_FALSE, vertex_size, (void*)offsetof(Vertex, normal));

    location = 2;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, 2, GL_FLOAT, GL_FALSE, vertex_size, (void*)offsetof(Vertex, tex_coords));

    location = 3;
    glEnableVertexAttribArray(location);
    glVertexAttribIPointer(location, MAX_N_BONES_PER_VERTEX, GL_INT, vertex_size, (void*)offsetof(Vertex, bone_idxs));

    location = 4;
    glEnableVertexAttribArray(location);
    glVertexAttribPointer(location, MAX_N_BONES_PER_VERTEX, GL_FLOAT, GL_FALSE, vertex_size,
        (void*)offsetof(Vertex, bone_weights));
}


bool
geom::is_mesh_valid(geom::Mesh *mesh)
{
    return mesh->vao > 0;
}


void
geom::destroy_mesh(geom::Mesh *mesh)
{
    glDeleteVertexArrays(1, &mesh->vao);
    glDeleteBuffers(1, &mesh->vbo);
    glDeleteBuffers(1, &mesh->ebo);
}


void
geom::make_plane(
    memory::Pool *memory_pool,
    u32 x_size, u32 z_size,
    u32 n_x_segments, u32 n_z_segments,
    u32 *n_vertices, u32 *n_indices,
    geom::Vertex **vertex_data, u32 **index_data
) {
    *n_vertices = 0;
    *n_indices = 0;

    u32 n_total_vertices = (n_x_segments + 1) * (n_z_segments + 1);
    u32 index_data_length = (n_x_segments) * (n_z_segments) * 6;

    *vertex_data = (geom::Vertex*)memory::push(memory_pool, sizeof(geom::Vertex) * n_total_vertices, "plane_vertex_data");
    *index_data = (u32*)memory::push(memory_pool, sizeof(u32) * index_data_length, "plane_index_data");

    for (u32 idx_x = 0; idx_x <= n_x_segments; idx_x++) {
        for (u32 idx_z = 0; idx_z <= n_z_segments; idx_z++) {
            f32 x_segment = (f32)idx_x / (f32)n_x_segments;
            f32 z_segment = (f32)idx_z / (f32)n_z_segments;
            f32 x_pos = x_segment * x_size - (x_size / 2);
            f32 y_pos = 0;
            f32 z_pos = z_segment * z_size - (z_size / 2);

            (*vertex_data)[(*n_vertices)++] = {
                .position = { x_pos, y_pos, z_pos },
                .normal = { 0.0f, 1.0f, 0.0f },
                .tex_coords = { x_segment, z_segment },
            };
        }
    }

    // NOTE: Counterclockwise winding order. I could swear this code is CW
    // order though. Not sure where the confusion happens.
    for (u32 idx_x = 0; idx_x < n_x_segments; idx_x++) {
        for (u32 idx_z = 0; idx_z < n_z_segments; idx_z++) {
            // This current vertex.
            (*index_data)[(*n_indices)++] = (idx_x * (n_z_segments + 1)) + idx_z;
            // Next row, right of this one.
            (*index_data)[(*n_indices)++] = ((idx_x + 1) * (n_z_segments + 1)) + idx_z + 1;
            // Next row, under this one.
            (*index_data)[(*n_indices)++] = ((idx_x + 1) * (n_z_segments + 1)) + idx_z;

            // This current vertex.
            (*index_data)[(*n_indices)++] = (idx_x * (n_z_segments + 1)) + idx_z;
            // This row, right of this one.
            (*index_data)[(*n_indices)++] = (idx_x * (n_z_segments + 1)) + idx_z + 1;
            // Next row, right of this one.
            (*index_data)[(*n_indices)++] = ((idx_x + 1) * (n_z_segments + 1)) + idx_z + 1;
        }
    }
}


void
geom::make_sphere(
    memory::Pool *memory_pool,
    u32 n_x_segments, u32 n_y_segments,
    u32 *n_vertices, u32 *n_indices,
    geom::Vertex **vertex_data, u32 **index_data
) {
    *n_vertices = 0;
    *n_indices = 0;

    u32 total_n_vertices = (n_x_segments + 1) * (n_y_segments + 1);
    u32 index_data_length = (n_x_segments + 1) * (n_y_segments) * 2;

    *vertex_data = (geom::Vertex*)memory::push(memory_pool, sizeof(geom::Vertex) * total_n_vertices, "sphere_vertex_data");
    *index_data = (u32*)memory::push(memory_pool, sizeof(u32) * index_data_length, "sphere_index_data");

    for (u32 y = 0; y <= n_y_segments; y++) {
        for (u32 x = 0; x <= n_x_segments; x++) {
            f32 x_segment = (f32)x / (f32)n_x_segments;
            f32 y_segment = (f32)y / (f32)n_y_segments;
            f32 x_pos = cos(x_segment * 2.0f * PI32) * sin(y_segment * PI32);
            f32 y_pos = cos(y_segment * PI32);
            f32 z_pos = sin(x_segment * 2.0f * PI32) * sin(y_segment * PI32);

            (*vertex_data)[(*n_vertices)++] = {
                .position = {x_pos, y_pos, z_pos},
                .normal = {x_pos, y_pos, z_pos},
                .tex_coords = {x_segment, y_segment},
            };
        }
    }

    for (u32 y = 0; y < n_y_segments; y++) {
        if (y % 2 == 0) {
            for (u32 x = 0; x <= n_x_segments; x++) {
                (*index_data)[(*n_indices)++] = (y + 1) * (n_x_segments + 1) + x;
                (*index_data)[(*n_indices)++] = y * (n_x_segments + 1) + x;
            }
        } else {
            for (i32 x = n_x_segments; x >= 0; x--) {
                (*index_data)[(*n_indices)++] = y * (n_x_segments + 1) + x;
                (*index_data)[(*n_indices)++] = (y + 1) * (n_x_segments + 1) + x;
            }
        }
    }
}
