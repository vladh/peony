// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "entities.hpp"
#include "spatial.hpp"

class physics {
public:
    static constexpr f32 PARALLEL_FACE_TOLERANCE = 1.0e-2;
    static constexpr f32 RELATIVE_TOLERANCE = 1.00f;
    static constexpr f32 ABSOLUTE_TOLERANCE = 0.10f;

    struct Component {
        entities::Handle entity_handle;
        spatial::Obb obb;
        spatial::Obb transformed_obb;
    };

    struct ComponentSet {
        Array<Component> components;
    };

    struct CollisionManifold {
        bool did_collide;
        Component *collidee;
        f32 sep_max;
        u32 axis;
        v3 normal;
    };

    struct RaycastResult {
        bool did_intersect;
        f32 distance;
    };

    struct RayCollisionResult {
        bool did_intersect;
        f32 distance;
        Component *collidee;
    };

    static RayCollisionResult find_ray_collision(
        spatial::Ray *ray,
        Component *physics_component_to_ignore_or_nullptr
    );
    static CollisionManifold find_collision(
        Component *self_physics,
        spatial::Component *self_spatial
    );
    static void update();

private:
    static spatial::Obb transform_obb(spatial::Obb obb, spatial::Component *spatial);
    static RaycastResult intersect_obb_ray(spatial::Obb *obb, spatial::Ray *ray);
    static bool is_component_valid(Component *physics_component);
    static v3 get_edge_contact_point(
        v3 a_edge_point,
        v3 a_axis,
        f32 a_axis_length,
        v3 b_edge_point,
        v3 b_axis,
        f32 b_axis_length,
        bool should_use_a_midpoint
    );
    static spatial::Face get_incident_face(m3 *cob, v3 e, v3 c, v3 n);
    static void get_reference_face_edges_and_basis(
        m3 *cob,
        v3 e,
        v3 c,
        v3 n,
        u32 axis,
        u32 clip_edges[4],
        m3 *reference_face_cob,
        v3 *reference_face_e
    );
    static u32 clip_faces(
        v3 reference_center, v3 reference_face_extents,
        u32 clip_edges[4], m3 reference_face_cob,
        spatial::Face incident_face,
        v3 clip_vertices[8], f32 clip_depths[8]
    );
    static void update_best_for_face_axis(
        f32 *best_sep, u32 *best_axis, v3 *best_normal,
        f32 sep, u32 axis, v3 normal
    );
    static void update_best_for_edge_axis(
        f32 *best_sep, u32 *best_axis, v3 *best_normal,
        f32 sep, u32 axis, v3 normal
    );
    static CollisionManifold intersect_obb_obb(
        spatial::Obb *a,
        spatial::Obb *b,
        spatial::Component *spatial_a,
        spatial::Component *spatial_b
    );
};
