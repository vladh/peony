// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "logs.hpp"
#include "util.hpp"


f64
util::random(f64 min, f64 max)
{
    u32 r = rand();
    f64 r_normalized = (f64)r / (f64)RAND_MAX;
    return min + ((r_normalized) * (max - min));
}


v3
util::aiVector3D_to_glm(aiVector3D *vec)
{
    return v3(vec->x, vec->y, vec->z);
}


quat
util::aiQuaternion_to_glm(aiQuaternion *rotation)
{
    return quat(rotation->w, rotation->x, rotation->y, rotation->z);
}


m4
util::aimatrix4x4_to_glm(aiMatrix4x4 *from)
{
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


f32
util::round_to_nearest_multiple(f32 n, f32 multiple_of)
{
    return (floor((n) / multiple_of) * multiple_of) + multiple_of;
}


f64
util::get_us_from_duration(chrono::duration<f64> duration)
{
    return chrono::duration_cast<chrono::duration<f64>>(duration).count();
}


v3
util::get_orthogonal_vector(v3 *v)
{
    if (v->z < v->x) {
        return v3(v->y, -v->x, 0.0f);
    } else {
        return v3(0.0f, -v->z, v->y);
    }
}


u32 util::kb_to_b(u32 value) { return value * 1024; }
u32 util::mb_to_b(u32 value) { return kb_to_b(value) * 1024; }
u32 util::gb_to_b(u32 value) { return mb_to_b(value) * 1024; }
u32 util::tb_to_b(u32 value) { return gb_to_b(value) * 1024; }
f32 util::b_to_kb(u32 value) { return value / 1024.0f; }
f32 util::b_to_mb(u32 value) { return b_to_kb(value) / 1024.0f; }
f32 util::b_to_gb(u32 value) { return b_to_mb(value) / 1024.0f; }
f32 util::b_to_tb(u32 value) { return b_to_gb(value) / 1024.0f; }
