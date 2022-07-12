// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "pack.hpp"
#include "constants.hpp"

class geom {
public:
    struct Vertex {
        v3 position;
        v3 normal;
        v2 tex_coords;
        u32 bone_idxs[MAX_N_BONES_PER_VERTEX];
        f32 bone_weights[MAX_N_BONES_PER_VERTEX];
    };

    struct Mesh {
        memory::Pool temp_memory_pool;
        m4 transform;
        char material_name[MAX_COMMON_NAME_LENGTH];
        pack::Pack indices_pack;
        u32 vao;
        u32 vbo;
        u32 ebo;
        GLenum mode;
        Vertex *vertices;
        u32 *indices;
        u32 n_vertices;
        u32 n_indices;
    };

    static void setup_mesh_vertex_buffers(
        Mesh *mesh,
        Vertex *vertex_data, u32 n_vertices,
        u32 *index_data, u32 n_indices
    );
    static bool is_mesh_valid(Mesh *mesh);
    static void destroy_mesh(Mesh *mesh);
    static void make_plane(
        memory::Pool *memory_pool,
        u32 x_size, u32 z_size,
        u32 n_x_segments, u32 n_z_segments,
        u32 *n_vertices, u32 *n_indices,
        Vertex **vertex_data, u32 **index_data
    );
    static void make_sphere(
        memory::Pool *memory_pool,
        u32 n_x_segments, u32 n_y_segments,
        u32 *n_vertices, u32 *n_indices,
        Vertex **vertex_data, u32 **index_data
    );
};
