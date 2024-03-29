// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "logs.hpp"
#include "gui.hpp"
#include "debugdraw.hpp"
#include "physics.hpp"
#include "intrinsics.hpp"


physics::State *physics::state = nullptr;


physics::RayCollisionResult
physics::find_ray_collision(
    spatial::Ray *ray,
    // TODO: Replace this with some kind of collision layers.
    physics::Component *physics_component_to_ignore_or_nullptr
) {
    each (candidate, *get_components()) {
        if (!is_component_valid(candidate)) {
            continue;
        }

        if (physics_component_to_ignore_or_nullptr == candidate) {
            continue;
        }

        RaycastResult raycast_result = intersect_obb_ray(&candidate->transformed_obb, ray);
        if (raycast_result.did_intersect) {
            return {
                .did_intersect = raycast_result.did_intersect,
                .distance = raycast_result.distance,
                .collidee = candidate,
            };
        }
    }

    return {};
}


physics::CollisionManifold
physics::find_collision(
    physics::Component *self_physics,
    spatial::Component *self_spatial
) {
    each (candidate_physics, *get_components()) {
        if (!is_component_valid(candidate_physics)) {
            continue;
        }
        spatial::Component *candidate_spatial = spatial::get_component(candidate_physics->entity_handle);
        if (!candidate_spatial) {
            logs::error("Could not get spatial::Component for candidate");
            return physics::CollisionManifold {};
        }

        if (self_physics == candidate_physics) {
            continue;
        }

        physics::CollisionManifold manifold = intersect_obb_obb(
            &self_physics->transformed_obb, &candidate_physics->transformed_obb, self_spatial, candidate_spatial);
        if (manifold.did_collide) {
            manifold.collidee = candidate_physics;
            return manifold;
        }
    }

    return physics::CollisionManifold {};
}


void
physics::update()
{
    each (physics_component, *get_components()) {
        if (!is_component_valid(physics_component)) {
            continue;
        }

        spatial::Component *spatial_component = spatial::get_component(physics_component->entity_handle);

        if (!spatial::is_spatial_component_valid(spatial_component)) {
            logs::warning("Tried to update physics component %d but it had no spatial component.",
                physics_component->entity_handle);
            continue;
        }

        physics_component->transformed_obb = transform_obb(physics_component->obb, spatial_component);
    }
}


Array<physics::Component> *
physics::get_components()
{
    return &physics::state->components;
}


physics::Component *
physics::get_component(entities::Handle entity_handle)
{
    return physics::state->components[entity_handle];
}


void
physics::init(physics::State *physics_state, memory::Pool *asset_memory_pool)
{
    physics::state = physics_state;
    physics::state->components = Array<physics::Component>(
        asset_memory_pool, MAX_N_ENTITIES, "physics_components", true, 1);
}


spatial::Obb
physics::transform_obb(spatial::Obb obb, spatial::Component *spatial)
{
    m3 rotation = glm::toMat3(normalize(spatial->rotation));
    obb.center = spatial->position + (rotation * (spatial->scale * obb.center));
    obb.x_axis = normalize(rotation * obb.x_axis);
    obb.y_axis = normalize(rotation * obb.y_axis);
    obb.extents *= spatial->scale;
    return obb;
}


physics::RaycastResult
physics::intersect_obb_ray(spatial::Obb *obb, spatial::Ray *ray)
{
    // Gabor Szauer, Game Physics Cookbook, “Raycast Oriented Bounding Box”
    v3 obb_z_axis = cross(obb->x_axis, obb->y_axis);

    // Get vector pointing from origin of ray to center of OBB
    v3 p = obb->center - ray->origin;

    // Project direction of ray onto each axis of OBB
    v3 f = v3(
        dot(obb->x_axis, ray->direction),
        dot(obb->y_axis, ray->direction),
        dot(obb_z_axis, ray->direction));

    // Project p into every axis of OBB
    v3 e = v3(
        dot(obb->x_axis, p),
        dot(obb->y_axis, p),
        dot(obb_z_axis, p));

    // Calculate slab intersection points for ray
    // `t is the distance along the ray (or “time” along the ray, as Szauer
    // calls it) that the intersection happens at.
    f32 t[6] = {};
    range_named (i, 0, 3) {
        if (f[i] == 0) {
            if (-e[i] - obb->extents[i] > 0 || -e[i] + obb->extents[i] < 0) {
                // If the ray is parallel to the slab being tested, and the origin
                // of the ray is not inside the slab, we have no hit.
                return {};
            }
            f[i] = 0.00001f; // Avoid division by zero
        }
        t[i * 2 + 0] = (e[i] + obb->extents[i]) / f[i]; // min
        t[i * 2 + 1] = (e[i] - obb->extents[i]) / f[i]; // max
    }

    // After the above loop, we've hit all three slabs. We now need to find the
    // largest minimum `t^{min}` and smallest maximum `t^{max}`.
    f32 tmin = max(
        max(min(t[0], t[1]), min(t[2], t[3])),
        min(t[4], t[5]));
    f32 tmax = min(
        min(max(t[0], t[1]), max(t[2], t[3])),
        max(t[4], t[5]));

    // If `tmax` < 0, the ray is intersecting the OBB in the negative direction.
    // This means the OBB is behind the origin of the ray, and this should not
    // count as an intersection.
    if (tmax < 0.0f) {
        return {};
    }

    // If `tmin` > `tmax`, the ray does not intersect the OBB.
    if (tmin > tmax) {
        return {};
    }

    // If `tmin` < 0, the ray started inside of the OBB. This means `tmax` is a
    // valid intersection.
    if (tmin < 0.0f) {
        return {
            .did_intersect = true,
            .distance = tmax,
        };
    }

    return {
        .did_intersect = true,
        .distance = tmin,
    };
}


bool
physics::is_component_valid(physics::Component *physics_component) {
    return physics_component->obb.extents.x > 0;
}


/*!
    This function gets the nearest contact point between two edges. It's used
    to determine a collision point for a box collision that has happened
    edge-to-edge.

    should_use_a_midpoint
    ---------------------
    If this is true, and the contact point is outside the edge (in the case of
    an edge-face contact) then we use a's midpoint, otherwise we use b's.

    Resources
    ---------
    This function is heavily based on code from Ian Millington's Cyclone Physics
    engine.

    idmillington/cyclone-physics/blob/master/src/collide_fine.cpp#contactPoint()
*/
v3
physics::get_edge_contact_point(
    v3 a_edge_point,
    v3 a_axis,
    f32 a_axis_length,
    v3 b_edge_point,
    v3 b_axis,
    f32 b_axis_length,
    bool should_use_a_midpoint
) {
    f32 a_axis_sqlen = length2(a_axis);
    f32 b_axis_sqlen = length2(b_axis);
    f32 a_b_axes_dotprod = dot(b_axis, a_axis);

    v3 a_ep_to_b_ep = a_edge_point - b_edge_point;
    f32 a_ep_projection = dot(a_axis, a_ep_to_b_ep);
    f32 b_ep_projection = dot(b_axis, a_ep_to_b_ep);

    f32 denom = a_axis_sqlen * b_axis_sqlen - a_b_axes_dotprod * a_b_axes_dotprod;

    // Zero denominator indicates parallel lines
    if (abs(denom) < 0.0001f) {
        return should_use_a_midpoint ? a_edge_point : b_edge_point;
    }

    f32 mua = (a_b_axes_dotprod * b_ep_projection - b_axis_sqlen * a_ep_projection) / denom;
    f32 mub = (a_axis_sqlen * b_ep_projection - a_b_axes_dotprod * a_ep_projection) / denom;

    // If either of the edges has the nearest point out of bounds, then the edges
    // aren't crossed, we have an edge-face contact. Our point is on the edge,
    // which we know from the should_use_a_midpoint parameter.
    if (mua > a_axis_length || mua < -a_axis_length || mub > b_axis_length || mub < -b_axis_length) {
        return should_use_a_midpoint ? a_edge_point : b_edge_point;
    } else {
        v3 contact_a_component = a_edge_point + a_axis * mua;
        v3 contact_b_component = b_edge_point + b_axis * mub;

        return contact_a_component * 0.5f + contact_b_component * 0.5f;
    }
}


spatial::Face
physics::get_incident_face(
    m3 *cob, // incident change of base
    v3 e, // incident extents
    v3 c, // incident center
    v3 n // incident normal
) {
    spatial::Face face;
    n = transpose(*cob) * n;
    v3 abs_n = abs(n);

    if (abs_n.x > abs_n.y && abs_n.x > abs_n.z) {
        if (n.x > 0.0f) {
            face = {
                .vertices = {
                    v3(e.x,  e.y, -e.z),
                    v3(e.x,  e.y,  e.z),
                    v3(e.x, -e.y,  e.z),
                    v3(e.x, -e.y, -e.z),
                },
            };
        } else {
            face = {
                .vertices = {
                    v3(-e.x, -e.y,  e.z),
                    v3(-e.x,  e.y,  e.z),
                    v3(-e.x,  e.y, -e.z),
                    v3(-e.x, -e.y, -e.z),
                },
            };
        }
    } else if (abs_n.y > abs_n.x && abs_n.y > abs_n.z) {
        if (n.y > 0.0f) {
            face = {
                .vertices = {
                    v3(-e.x,  e.y,  e.z),
                    v3( e.x,  e.y,  e.z),
                    v3( e.x,  e.y, -e.z),
                    v3(-e.x,  e.y, -e.z),
                },
            };
        } else {
            face = {
                .vertices = {
                    v3( e.x, -e.y,  e.z),
                    v3(-e.x, -e.y,  e.z),
                    v3(-e.x, -e.y, -e.z),
                    v3( e.x, -e.y, -e.z),
                },
            };
        }
    } else {
        if (n.z > 0.0f) {
            face = {
                .vertices = {
                    v3(-e.x,  e.y,  e.z),
                    v3(-e.x, -e.y,  e.z),
                    v3( e.x, -e.y,  e.z),
                    v3( e.x,  e.y,  e.z),
                },
            };
        } else {
            face = {
                .vertices = {
                    v3( e.x, -e.y, -e.z),
                    v3(-e.x, -e.y, -e.z),
                    v3(-e.x,  e.y, -e.z),
                    v3( e.x,  e.y, -e.z),
                },
            };
        }
    }

    range (0, 4) {
        face.vertices[idx] = *cob * face.vertices[idx] + c;
    }

    return face;
}


void
physics::get_reference_face_edges_and_basis(
    m3 *cob, // object change of base
    v3 e, // object extents
    v3 c, // object center
    v3 n, // collision normal
    u32 axis, // axis of separation
    u32 clip_edges[4], // the indices of the reference face edges
    m3 *reference_face_cob, // the change of basis of the reference face
    v3 *reference_face_e // the extents of the reference face
) {
    n = transpose(*cob) * n;

    if (axis >= 3) {
        axis -= 3;
    }

    if (axis == 0) {
        if (n.x > 0.0f) {
            clip_edges[0] = 1;
            clip_edges[1] = 8;
            clip_edges[2] = 7;
            clip_edges[3] = 9;
            row(*reference_face_cob, 0, row(*cob, 1));
            row(*reference_face_cob, 1, row(*cob, 2));
            row(*reference_face_cob, 2, row(*cob, 0));
            *reference_face_e = v3(e.y, e.z, e.x);
        } else {
            clip_edges[0] = 11;
            clip_edges[1] = 3;
            clip_edges[2] = 10;
            clip_edges[3] = 5;
            row(*reference_face_cob, 0,  row(*cob, 2));
            row(*reference_face_cob, 1,  row(*cob, 1));
            row(*reference_face_cob, 2, -row(*cob, 0));
            *reference_face_e = v3(e.z, e.y, e.x);
        }
    } else if (axis == 1) {
        if (n.y > 0.0f) {
            clip_edges[0] = 0;
            clip_edges[1] = 1;
            clip_edges[2] = 2;
            clip_edges[3] = 3;
            row(*reference_face_cob, 0, row(*cob, 2));
            row(*reference_face_cob, 1, row(*cob, 0));
            row(*reference_face_cob, 2, row(*cob, 1));
            *reference_face_e = v3(e.z, e.x, e.y);
        } else {
            clip_edges[0] = 4;
            clip_edges[1] = 5;
            clip_edges[2] = 6;
            clip_edges[3] = 7;
            row(*reference_face_cob, 0,  row(*cob, 2));
            row(*reference_face_cob, 1, -row(*cob, 0));
            row(*reference_face_cob, 2, -row(*cob, 1));
            *reference_face_e = v3(e.z, e.x, e.y);
        }
    } else if (axis == 2) {
        if (n.z > 0.0f) {
            clip_edges[0] = 11;
            clip_edges[1] = 4;
            clip_edges[2] = 8;
            clip_edges[3] = 0;
            row(*reference_face_cob, 0, -row(*cob, 1));
            row(*reference_face_cob, 1,  row(*cob, 0));
            row(*reference_face_cob, 2,  row(*cob, 2));
            *reference_face_e = v3(e.y, e.x, e.z);
        } else {
            clip_edges[0] = 6;
            clip_edges[1] = 10;
            clip_edges[2] = 2;
            clip_edges[3] = 9;
            row(*reference_face_cob, 0, -row(*cob, 1));
            row(*reference_face_cob, 1, -row(*cob, 0));
            row(*reference_face_cob, 2, -row(*cob, 2));
            *reference_face_e = v3(e.y, e.x, e.z);
        }
    }
}


u32
physics::clip_faces(
    v3 reference_center, v3 reference_face_extents,
    u32 clip_edges[4], m3 reference_face_cob,
    spatial::Face incident_face,
    v3 clip_vertices[8], f32 clip_depths[8]
) {
    return 0;
}


void
physics::update_best_for_face_axis(
    f32 *best_sep, u32 *best_axis, v3 *best_normal,
    f32 sep, u32 axis, v3 normal
) {
    if (sep > *best_sep) {
        *best_sep = sep;
        *best_axis = axis;
        *best_normal = normal;
    }
}


void
physics::update_best_for_edge_axis(
    f32 *best_sep, u32 *best_axis, v3 *best_normal,
    f32 sep, u32 axis, v3 normal
) {
    f32 normal_len = length(normal);
    sep /= normal_len;
    if (sep > *best_sep) {
        *best_sep = sep;
        *best_axis = axis;
        *best_normal = normal / normal_len;
    }
}


/*!
    This function implements collision detection between two OBBs.

    We're using the separating axis test (SAT) to check which axes, if any,
    separates the two.

    For manifold generation, we're using the methods described by Dirk Gregorius,
    namely Sutherland-Hodgman clipping for face-something, and "just find the
    closes two points on the edges" for edge-edge.

    A note about normal calculation for the cross axes
    --------------------------------------------------
    Normally, we would calculate the normal as the axis we're using,
    so the cross product between the a axis and the b axis. We're not
    actually calculating this directly for SAT, because we're using the r
    matrix as a way around this. However, we do need this axis for the normal.
    Randy Gaul calculates a normal from the r matrix, which I have included
    as a comment. However, this is not orthogonal to both a's axis and b's axis.
    This might still be fine but I've left the cross product in, to be safe.
    We might look into using the r matrix method as an optimisation.

    Resources
    ---------
    * Christer Ericson, Real-Time Collision Detection, 4.4
    * Dirk Gregorius's GDC 2013 and GDC 2015 talks
    * Randy Gaul's blog post "Deriving OBB to OBB Intersection and Manifold Generation"
    * Ian Millington's Cyclone Physics engine (but not for face-something!)
*/
physics::CollisionManifold
physics::intersect_obb_obb(
    spatial::Obb *a,
    spatial::Obb *b,
    spatial::Component *spatial_a,
    spatial::Component *spatial_b
) {
    // The radius from a/b's center to its outer vertex
    f32 a_radius, b_radius;
    // The distance between a and b
    f32 a_to_b;
    // The separation between a and b
    f32 sep;
    // The rotation matrix expression b in a's coordinate frame
    m3 r;
    // abs(r) is used in a lot of calculations so we precompute it
    m3 abs_r;
    // We need to keep track of the normal on the edge axes
    v3 normal;

    v3 a_axes[3] = { a->x_axis, a->y_axis, cross(a->x_axis, a->y_axis) };
    v3 b_axes[3] = { b->x_axis, b->y_axis, cross(b->x_axis, b->y_axis) };

    // Change basis into world space (cob = change of base)
    m3 a_cob = m3(a_axes[0], a_axes[1], a_axes[2]);
    m3 b_cob = m3(b_axes[0], b_axes[1], b_axes[2]);

    // Compute rotation matrix expressing b in a's coordinate frame
    range_named (i, 0, 3) {
        range_named (j, 0, 3) {
            r[i][j] = dot(a_axes[i], b_axes[j]);
        }
    }

    // Compute translation vector
    v3 t_translation = b->center - a->center;
    gui::log(
        "a: center (%f, %f, %f) extents (%f, %f, %f)",
        a->center.x, a->center.y, a->center.z,
        a->extents[0], a->extents[1], a->extents[2]
    );
    gui::log(
        "b: center (%f, %f, %f) extents (%f, %f, %f)",
        b->center.x, b->center.y, b->center.z,
        b->extents[0], b->extents[1], b->extents[2]
    );

    // Bring translation into a's coordinate frame
    v3 t = v3(
        dot(t_translation, a_axes[0]),
        dot(t_translation, a_axes[1]),
        dot(t_translation, a_axes[2])
    );

    // If the two OBBs share one axis, we can skip checking their cross product
    // axes altogether. At the very least, if e.g. a.x and b.x are parallel,
    // their cross product will give us something we can't use. I'm not sure why
    // we're not skipping the specific axes specifically, and we're skipping
    // everything instead.
    bool do_obbs_share_one_axis = false;

    // Compute common subexpressions. Add in an epsilon term to counteract
    // arithmetic errors when two edges are parallel and their cross product
    // is (near) null.
    range_named (i, 0, 3) {
        range_named (j, 0, 3) {
            abs_r[i][j] = abs(r[i][j]) + physics::PARALLEL_FACE_TOLERANCE;
            if (abs_r[i][j] >= 1.0f) {
                do_obbs_share_one_axis = true;
            }
        }
    }

    f32 a_face_max_sep = -FLT_MAX;
    u32 a_face_best_axis = 0;
    v3 a_face_best_normal = v3(0.0f);
    f32 b_face_max_sep = -FLT_MAX;
    u32 b_face_best_axis = 0;
    v3 b_face_best_normal = v3(0.0f);
    f32 edge_max_sep = -FLT_MAX;
    u32 edge_best_axis = 0;
    v3 edge_best_normal = v3(0.0f);

    // Test a's face axes (a.x, a.y, a.z)
    range_named (i, 0, 3) {
        a_radius = a->extents[i];
        b_radius = b->extents[0] * abs_r[i][0] +
            b->extents[1] * abs_r[i][1] +
            b->extents[2] * abs_r[i][2];
        a_to_b = abs(t[i]);
        sep = a_to_b - (a_radius + b_radius);
        if (sep > 0) { return physics::CollisionManifold {}; }
        update_best_for_face_axis(
            &a_face_max_sep, &a_face_best_axis, &a_face_best_normal, sep, i, a_axes[i]);
    }

    // Test b's face axes (b.x, b.y, b.z)
    range_named (i, 0, 3) {
        a_radius = a->extents[0] * abs_r[0][i] +
            a->extents[1] * abs_r[1][i] +
            a->extents[2] * abs_r[2][i];
        b_radius = b->extents[i];
        a_to_b = abs(t[0] * r[0][i] + t[1] * r[1][i] + t[2] * r[2][i]);
        sep = a_to_b - (a_radius + b_radius);
        if (sep > 0) { return physics::CollisionManifold {}; }
        update_best_for_face_axis(
            &b_face_max_sep, &b_face_best_axis, &b_face_best_normal, sep, 3 + i, b_axes[i]);
    }

    if (!do_obbs_share_one_axis) {
        // Test cross axes (a[i] x b[j])
        range_named(i, 0, 3) {
            range_named(j, 0, 3) {
                // These numbers look really crazy, but it's not so bad if you look at
                // the table they come from.
                // See Christer Ericson, Real-Time Collision Detection, Table 4.1
                a_radius =
                    a->extents[i == 0 ? 1 : 0] * abs_r[i < 2 ? 2 : 1][j] +
                    a->extents[i < 2 ? 2 : 1] * abs_r[i == 0 ? 1 : 0][j];
                b_radius =
                    b->extents[j == 0 ? 1 : 0] * abs_r[i][j < 2 ? 2 : 1] +
                    b->extents[j < 2 ? 2 : 1] * abs_r[i][j == 0 ? 1 : 0];
                a_to_b = abs(
                    t[(2 + i) % 3] * r[(1 + i) % 3][j] -
                    t[(1 + i) % 3] * r[(2 + i) % 3][j]
                );
                sep = a_to_b - (a_radius + b_radius);
                if (sep > 0) { return physics::CollisionManifold {}; }
                normal = normalize(cross(a_axes[i], b_axes[j]));
                update_best_for_edge_axis(
                    &edge_max_sep, &edge_best_axis, &edge_best_normal, sep, 6 + i + j, normal);
            }
        }
    }

    // Find the best option for the face cases
    f32 face_max_sep;
    u32 face_best_axis;
    if (a_face_max_sep > b_face_max_sep) {
        face_max_sep = a_face_max_sep;
        face_best_axis = a_face_best_axis;
    } else {
        face_max_sep = b_face_max_sep;
        face_best_axis = b_face_best_axis;
    }

    // TODO: Remove this debugging code
    {
        v3 face_best_normal;
        if (a_face_max_sep > b_face_max_sep) {
            face_best_normal = a_face_best_normal;
        } else {
            face_best_normal = b_face_best_normal;
        }
        gui::log("a_face_max_sep %f", a_face_max_sep);
        gui::log("b_face_max_sep %f", b_face_max_sep);
        gui::log(
            "(face_max_sep (real %f) (adjusted %f) (face_best_axis %d) (face_best_normal %f %f %f))",
            face_max_sep,
            face_max_sep + physics::ABSOLUTE_TOLERANCE,
            face_best_axis,
            face_best_normal.x, face_best_normal.y, face_best_normal.z);
        gui::log(
            "(edge_max_sep (real %f) (adjusted %f) (edge_best_axis %d) (edge_best_normal %f %f %f))",
            edge_max_sep,
            edge_max_sep * physics::RELATIVE_TOLERANCE,
            edge_best_axis,
            edge_best_normal.x, edge_best_normal.y, edge_best_normal.z);
    }

    // Set manifold to our best option while taking tolerances into account
    // We use an artificial axis bias to improve frame coherence
    // (i.e. stop things from jumping between edge and face in nonsensical ways)
    physics::CollisionManifold manifold;
    if (edge_max_sep * physics::RELATIVE_TOLERANCE > face_max_sep + physics::ABSOLUTE_TOLERANCE) {
        manifold.sep_max = edge_max_sep;
        manifold.axis = edge_best_axis;
        manifold.normal = edge_best_normal;
    } else {
        if (b_face_max_sep * physics::RELATIVE_TOLERANCE > a_face_max_sep + physics::ABSOLUTE_TOLERANCE) {
            manifold.sep_max = b_face_max_sep;
            manifold.axis = b_face_best_axis;
            manifold.normal = b_face_best_normal;
        } else {
            manifold.sep_max = a_face_max_sep;
            manifold.axis = a_face_best_axis;
            manifold.normal = a_face_best_normal;
        }
    }

    // Correct normal direction
    if (dot(manifold.normal, t_translation) < 0.0f) {
        manifold.normal = -manifold.normal;
    }

    if (manifold.axis < 6) {
        // spatial::Face-something collision
        v3 reference_extents, incident_extents, reference_center, incident_center;
        m3 reference_cob, incident_cob;

        if (manifold.axis < 3) {
            manifold.normal = -manifold.normal;
            reference_extents = a->extents;
            reference_cob = a_cob;
            reference_center = a->center;
            incident_extents = b->extents;
            incident_cob = b_cob;
            incident_center = b->center;
        } else {
            reference_extents = b->extents;
            reference_cob = b_cob;
            reference_center = b->center;
            incident_extents = a->extents;
            incident_cob = a_cob;
            incident_center = a->center;
        }

        spatial::Face incident_face = get_incident_face(&incident_cob, incident_extents, incident_center, manifold.normal);

        u32 clip_edges[4];
        m3 reference_face_cob;
        v3 reference_face_extents;
        get_reference_face_edges_and_basis(
            &reference_cob, reference_extents, reference_center, manifold.normal,
            manifold.axis, clip_edges, &reference_face_cob, &reference_face_extents);

        u32 n_clip_vertices;
        v3 clip_vertices[8];
        f32 clip_depths[8];
        n_clip_vertices = clip_faces(
            reference_center, reference_face_extents,
            clip_edges, reference_face_cob,
            incident_face,
            clip_vertices, clip_depths);

        debugdraw::draw_quad(
            incident_face.vertices[0],
            incident_face.vertices[1],
            incident_face.vertices[2],
            incident_face.vertices[3],
            v4(0.0f, 1.0f, 0.0f, 1.0f));
        debugdraw::draw_point(
            incident_face.vertices[0],
            0.1f,
            v4(0.0f, 1.0f, 0.0f, 1.0f));
        debugdraw::draw_point(
            incident_face.vertices[1],
            0.1f,
            v4(0.0f, 1.0f, 0.0f, 1.0f));
        debugdraw::draw_point(
            incident_face.vertices[2],
            0.1f,
            v4(0.0f, 1.0f, 0.0f, 1.0f));
        debugdraw::draw_point(
            incident_face.vertices[3],
            0.1f,
            v4(0.0f, 1.0f, 0.0f, 1.0f));
    } else {
        // Edge-edge collision
        u32 edge_axis = manifold.axis - 6;
        u32 a_axis = edge_axis / 3;
        u32 b_axis = edge_axis % 3;

        v3 a_edge_point = a->extents;
        v3 b_edge_point = b->extents;
        range_named (i, 0, 3) {
            if (i == a_axis) {
                a_edge_point[i] = 0;
            } else if (dot(a_axes[i], manifold.normal) < 0) {
                a_edge_point[i] = -a_edge_point[i];
            }

            if (i == b_axis) {
                b_edge_point[i] = 0;
            } else if (dot(b_axes[i], manifold.normal) > 0) {
                b_edge_point[i] = -b_edge_point[i];
            }
        }

        a_edge_point = a_cob * a_edge_point + a->center;
        b_edge_point = b_cob * b_edge_point + b->center;

        v3 contact_point = get_edge_contact_point(
            a_edge_point,
            a_axes[a_axis],
            a->extents[a_axis],
            b_edge_point,
            b_axes[b_axis],
            b->extents[b_axis],
            face_best_axis >= 3);
        debugdraw::draw_point(
            contact_point,
            0.1f,
            v4(0.0f, 1.0f, 0.0f, 1.0f));
    }

    // Since no separating axis is found, the OBBs must be intersecting
    // NOTE: manifold.collidee should be filled in by the caller.
    manifold.did_collide = true;
    return manifold;
}
