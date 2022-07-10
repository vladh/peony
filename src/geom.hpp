// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "constants.hpp"

class geom {
public:
    struct Vertex {
        v3 position;
        v3 normal;
        v2 tex_coords;
        uint32 bone_idxs[MAX_N_BONES_PER_VERTEX];
        real32 bone_weights[MAX_N_BONES_PER_VERTEX];
    };

    struct Mesh {
        MemoryPool temp_memory_pool;
        m4 transform;
        char material_name[MAX_COMMON_NAME_LENGTH];
        Pack indices_pack;
        uint32 vao;
        uint32 vbo;
        uint32 ebo;
        GLenum mode;
        Vertex *vertices;
        uint32 *indices;
        uint32 n_vertices;
        uint32 n_indices;
    };
};
