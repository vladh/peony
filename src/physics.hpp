// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "entities.hpp"
#include "spatial.hpp"

class physics {
public:
    static constexpr real32 PARALLEL_FACE_TOLERANCE = 1.0e-2;
    static constexpr real32 RELATIVE_TOLERANCE = 1.00f;
    static constexpr real32 ABSOLUTE_TOLERANCE = 0.10f;

    struct Component {
        entities::Handle entity_handle;
        Obb obb;
        Obb transformed_obb;
    };

    struct ComponentSet {
        Array<Component> components;
    };

    struct CollisionManifold {
        bool32 did_collide;
        Component *collidee;
        real32 sep_max;
        uint32 axis;
        v3 normal;
    };

    struct RaycastResult {
        bool32 did_intersect;
        real32 distance;
    };

    struct RayCollisionResult {
        bool32 did_intersect;
        real32 distance;
        Component *collidee;
    };

    static RayCollisionResult find_ray_collision(
        Ray *ray,
        Component *physics_component_to_ignore_or_nullptr
    );
    static CollisionManifold find_collision(
        Component *self_physics,
        spatial::Component *self_spatial
    );
    static void update_components();

private:
    static Obb transform_obb(Obb obb, spatial::Component *spatial);
    static RaycastResult intersect_obb_ray(Obb *obb, Ray *ray);
    static bool is_component_valid(Component *physics_component);
    static v3 get_edge_contact_point(
        v3 a_edge_point,
        v3 a_axis,
        real32 a_axis_length,
        v3 b_edge_point,
        v3 b_axis,
        real32 b_axis_length,
        bool32 should_use_a_midpoint
    );
    static Face get_incident_face(m3 *cob, v3 e, v3 c, v3 n);
    static void get_reference_face_edges_and_basis(
        m3 *cob,
        v3 e,
        v3 c,
        v3 n,
        uint32 axis,
        uint32 clip_edges[4],
        m3 *reference_face_cob,
        v3 *reference_face_e
    );
    static uint32 clip_faces(
        v3 reference_center, v3 reference_face_extents,
        uint32 clip_edges[4], m3 reference_face_cob,
        Face incident_face,
        v3 clip_vertices[8], real32 clip_depths[8]
    );
    static void update_best_for_face_axis(
        real32 *best_sep, uint32 *best_axis, v3 *best_normal,
        real32 sep, uint32 axis, v3 normal
    );
    static void update_best_for_edge_axis(
        real32 *best_sep, uint32 *best_axis, v3 *best_normal,
        real32 sep, uint32 axis, v3 normal
    );
    static CollisionManifold intersect_obb_obb(
        Obb *a,
        Obb *b,
        spatial::Component *spatial_a,
        spatial::Component *spatial_b
    );
};
