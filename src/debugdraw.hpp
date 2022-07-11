// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "spatial.hpp"
#include "shaders.hpp"
#include "types.hpp"

class debugdraw {
public:
    static constexpr uint32 MAX_N_VERTICES = 2048;
    static constexpr uint32 VERTEX_LENGTH = 7;
    static constexpr size_t VERTEX_SIZE = sizeof(real32) * VERTEX_LENGTH;

    struct DebugDrawVertex {
        v3 position;
        v4 color;
    };

    struct State {
        shaders::Asset shader_asset;
        uint32 vao;
        uint32 vbo;
        DebugDrawVertex vertices[MAX_N_VERTICES];
        uint32 n_vertices_pushed;
    };

    static void draw_line(v3 start_pos, v3 end_pos, v4 color);
    static void draw_ray(Ray *ray, real32 length, v4 color);
    static void draw_quad(
        v3 p1, // clockwise: top left
        v3 p2, // top right
        v3 p3, // bottom right
        v3 p4, // bottom left
        v4 color
    );
    static void draw_box(
        v3 p1, // clockwise top face: top left
        v3 p2, // top right
        v3 p3, // bottom right
        v3 p4, // top left
        v3 p5, // clockwise bottom face: top left
        v3 p6, // top right
        v3 p7, // bottom right
        v3 p8, // top left
        v4 color
    );
    static void draw_obb(Obb *obb, v4 color);
    static void draw_point(v3 position, real32 size, v4 color);
    static void clear();
    static void render();
    static void init(debugdraw::State *debug_draw_state, MemoryPool *memory_pool);

private:
    static void push_vertices(DebugDrawVertex vertices[], uint32 n_vertices);

    static debugdraw::State *state;
};
