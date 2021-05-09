Obb Physics::transform_obb(Obb obb, SpatialComponent *spatial) {
  m3 rotation = glm::toMat3(normalize(spatial->rotation));
  obb.center = spatial->position + (rotation * (spatial->scale * obb.center));
  obb.x_axis = normalize(rotation * obb.x_axis);
  obb.y_axis = normalize(rotation * obb.y_axis);
  obb.extents *= spatial->scale;
  return obb;
}


RaycastResult Physics::intersect_obb_ray(Obb *obb, Ray *ray) {
  // Gabor Szauer, Game Physics Cookbook, “Raycast Oriented Bounding Box”
  v3 obb_z_axis = cross(obb->x_axis, obb->y_axis);

  // Get vector pointing from origin of ray to center of OBB
  v3 p = obb->center - ray->origin;

  // Project direction of ray onto each axis of OBB
  v3 f = v3(
    dot(obb->x_axis, ray->direction),
    dot(obb->y_axis, ray->direction),
    dot(obb_z_axis, ray->direction)
  );

  // Project p into every axis of OBB
  v3 e = v3(
    dot(obb->x_axis, p),
    dot(obb->y_axis, p),
    dot(obb_z_axis, p)
  );

  // Calculate slab intersection points for ray
  // `t is the distance along the ray (or “time” along the ray, as Szauer
  // calls it) that the intersection happens at.
  real32 t[6] = {};
  for_range_named (i, 0, 3) {
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
  real32 tmin = max(
    max(
      min(t[0], t[1]),
      min(t[2], t[3])
    ),
    min(t[4], t[5])
  );
  real32 tmax = min(
    min(
      max(t[0], t[1]),
      max(t[2], t[3])
    ),
    max(t[4], t[5])
  );

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


RayCollisionResult Physics::find_ray_collision(
  Ray *ray,
  // TODO: Replace this with some kind of collision layers.
  PhysicsComponent *physics_component_to_ignore_or_nullptr,
  PhysicsComponentSet *physics_component_set
) {
  for_each (candidate, physics_component_set->components) {
    if (!Entities::is_physics_component_valid(candidate)) {
      continue;
    }

    if (physics_component_to_ignore_or_nullptr == candidate) {
      continue;
    }

    RaycastResult raycast_result = Physics::intersect_obb_ray(
      &candidate->transformed_obb, ray
    );
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


void Physics::update_manifold_for_face_axis(
  CollisionManifold *manifold,
  real32 sep, uint32 axis, v3 normal
) {
  if (sep > manifold->sep_max) {
    manifold->sep_max = sep;
    manifold->axis = axis;
    manifold->normal = normal;
  }
}


void Physics::update_manifold_for_edge_axis(
  CollisionManifold *manifold,
  real32 sep, uint32 axis, v3 normal
) {
  real32 normal_len = length(normal);
  sep /= normal_len;
  if (sep > manifold->sep_max) {
    manifold->sep_max = sep;
    manifold->axis = axis;
    manifold->normal = normal / normal_len;
  }
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
v3 Physics::get_edge_contact_point(
  v3 a_edge_point,
  v3 a_axis,
  real32 a_axis_length,
  v3 b_edge_point,
  v3 b_axis,
  real32 b_axis_length,
  bool32 should_use_a_midpoint
) {
  real32 a_axis_sqlen = length2(a_axis);
  real32 b_axis_sqlen = length2(b_axis);
  real32 a_b_axes_dotprod = dot(b_axis, a_axis);

  v3 a_ep_to_b_ep = a_edge_point - b_edge_point;
  real32 a_ep_projection = dot(a_axis, a_ep_to_b_ep);
  real32 b_ep_projection = dot(b_axis, a_ep_to_b_ep);

  real32 denom = a_axis_sqlen * b_axis_sqlen - a_b_axes_dotprod * a_b_axes_dotprod;

  // Zero denominator indicates parallel lines
  if (abs(denom) < 0.0001f) {
    return should_use_a_midpoint ? a_edge_point : b_edge_point;
  }

  real32 mua = (
    a_b_axes_dotprod * b_ep_projection - b_axis_sqlen * a_ep_projection
  ) / denom;
  real32 mub = (
    a_axis_sqlen * b_ep_projection - a_b_axes_dotprod * a_ep_projection
  ) / denom;

  // If either of the edges has the nearest point out of bounds, then the edges
  // aren't crossed, we have an edge-face contact. Our point is on the edge,
  // which we know from the should_use_a_midpoint parameter.
  if (
    mua > a_axis_length ||
    mua < -a_axis_length ||
    mub > b_axis_length ||
    mub < -b_axis_length
  ) {
    return should_use_a_midpoint ? a_edge_point : b_edge_point;
  } else {
    v3 contact_a_component = a_edge_point + a_axis * mua;
    v3 contact_b_component = b_edge_point + b_axis * mub;

    return contact_a_component * 0.5f + contact_b_component * 0.5f;
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
* Randy Gaul's blog post
  "Deriving OBB to OBB Intersection and Manifold Generation"
* Ian Millington's Cyclone Physics engine (but not for face-something!)
*/
CollisionManifold Physics::intersect_obb_obb(
  Obb *a,
  Obb *b,
  SpatialComponent *spatial_a,
  SpatialComponent *spatial_b
) {
  CollisionManifold manifold = {.sep_max = -FLT_MAX};
  // The radius from a/b's center to its outer vertex
  real32 a_radius, b_radius;
  // The distance between a and b
  real32 a_to_b;
  // The separation between a and b
  real32 sep;
  // The rotation matrix expression b in a's coordinate frame
  m3 r;
  // abs(r) is used in a lot of calculations so we precompute it
  m3 abs_r;
  // We need to keep track of the normal on the edge axes
  v3 normal;

  v3 a_axes[3] = {
    a->x_axis, a->y_axis, cross(a->x_axis, a->y_axis)
  };
  v3 b_axes[3] = {
    b->x_axis, b->y_axis, cross(b->x_axis, b->y_axis)
  };

  // Compute rotation matrix expressing b in a's coordinate frame
  for_range_named (i, 0, 3) {
    for_range_named (j, 0, 3) {
      r[i][j] = dot(a_axes[i], b_axes[j]);
    }
  }

  // Compute translation vector
  v3 t_translation = b->center - a->center;

  // Bring translation into a's coordinate frame
  v3 t = v3(
    dot(t_translation, a_axes[0]),
    dot(t_translation, a_axes[1]),
    dot(t_translation, a_axes[2])
  );

  bool32 do_obbs_share_one_axis = false;

  // Compute common subexpressions. Add in an epsilon term to counteract
  // arithmetic errors when two edges are parallel and their cross product
  // is (near) null.
  for_range_named (i, 0, 3) {
    for_range_named (j, 0, 3) {
      abs_r[i][j] = abs(r[i][j]) + FLT_EPSILON;
      if (abs_r[i][j] >= 1.0f) {
        do_obbs_share_one_axis = true;
      }
    }
  }

  // Test axes a.x, a.y, a.z
  for_range_named (i, 0, 3) {
    a_radius = a->extents[i];
    b_radius = b->extents[0] * abs_r[i][0] +
      b->extents[1] * abs_r[i][1] +
      b->extents[2] * abs_r[i][2];
    a_to_b = abs(t[i]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    update_manifold_for_face_axis(&manifold, sep, i, a_axes[i]);
  }

  // Test axes b.x, b.y, b.z
  for_range_named (i, 0, 3) {
    a_radius = a->extents[0] * abs_r[0][i] +
      a->extents[1] * abs_r[1][i] +
      a->extents[2] * abs_r[2][i];
    b_radius = b->extents[i];
    a_to_b = abs(t[0] * r[0][i] + t[1] * r[1][i] + t[2] * r[2][i]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    update_manifold_for_face_axis(&manifold, sep, 3 + i, b_axes[i]);
  }

  // Out of the first 6 axes, keep track of the best one
  uint32 face_axis_with_max_separation = manifold.axis;

  if (do_obbs_share_one_axis) {
    // If the two OBBs share one axis, we can skip checking their cross product
    // axes altogether.
    // NOTE: (@vladh) It's not 100% clear to me why this is.
  } else {
    // Test axis a.x x b.x
    a_radius = a->extents[1] * abs_r[2][0] + a->extents[2] * abs_r[1][0];
    b_radius = b->extents[1] * abs_r[0][2] + b->extents[2] * abs_r[0][1];
    a_to_b = abs(t[2] * r[1][0] - t[1] * r[2][0]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    // normal = v3(0.0f, -r[2][0], r[1][0]);
    normal = normalize(cross(a_axes[0], b_axes[0]));
    update_manifold_for_edge_axis(&manifold, sep, 6, normal);

    // Test axis a.x x b.y
    a_radius = a->extents[1] * abs_r[2][1] + a->extents[2] * abs_r[1][1];
    b_radius = b->extents[0] * abs_r[0][2] + b->extents[2] * abs_r[0][0];
    a_to_b = abs(t[2] * r[1][1] - t[1] * r[2][1]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    // normal = v3(0.0f, r[2][1], -r[1][1]);
    normal = normalize(cross(a_axes[0], b_axes[1]));
    update_manifold_for_edge_axis(&manifold, sep, 7, normal);

    // Test axis a.x x b.z
    a_radius = a->extents[1] * abs_r[2][2] + a->extents[2] * abs_r[1][2];
    b_radius = b->extents[0] * abs_r[0][1] + b->extents[1] * abs_r[0][0];
    a_to_b = abs(t[2] * r[1][2] - t[1] * r[2][2]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    // normal = v3(0.0f, -r[2][2], r[1][2]);
    normal = normalize(cross(a_axes[0], b_axes[2]));
    update_manifold_for_edge_axis(&manifold, sep, 8, normal);

    // Test axis a.y x b.x
    a_radius = a->extents[0] * abs_r[2][0] + a->extents[2] * abs_r[0][0];
    b_radius = b->extents[1] * abs_r[1][2] + b->extents[2] * abs_r[1][1];
    a_to_b = abs(t[0] * r[2][0] - t[2] * r[0][0]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    // normal = v3(r[2][0], 0.0f, -r[0][0]);
    normal = normalize(cross(a_axes[1], b_axes[0]));
    update_manifold_for_edge_axis(&manifold, sep, 9, normal);

    // Test axis a.y x b.y
    a_radius = a->extents[0] * abs_r[2][1] + a->extents[2] * abs_r[0][1];
    b_radius = b->extents[0] * abs_r[1][2] + b->extents[2] * abs_r[1][0];
    a_to_b = abs(t[0] * r[2][1] - t[2] * r[0][1]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    // normal = v3(r[2][1], 0.0f, -r[0][1]);
    normal = normalize(cross(a_axes[1], b_axes[1]));
    update_manifold_for_edge_axis(&manifold, sep, 10, normal);

    // Test axis a.y x b.z
    a_radius = a->extents[0] * abs_r[2][2] + a->extents[2] * abs_r[0][2];
    b_radius = b->extents[0] * abs_r[1][1] + b->extents[1] * abs_r[1][0];
    a_to_b = abs(t[0] * r[2][2] - t[2] * r[0][2]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    // normal = v3(r[2][2], 0.0f, -r[0][2]);
    normal = normalize(cross(a_axes[1], b_axes[2]));
    update_manifold_for_edge_axis(&manifold, sep, 11, normal);

    // Test axis a.z x b.x
    a_radius = a->extents[0] * abs_r[1][0] + a->extents[1] * abs_r[0][0];
    b_radius = b->extents[1] * abs_r[2][2] + b->extents[2] * abs_r[2][1];
    a_to_b = abs(t[1] * r[0][0] - t[0] * r[1][0]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    // normal = v3(-r[1][0], r[0][0], 0.0f);
    normal = normalize(cross(a_axes[2], b_axes[0]));
    update_manifold_for_edge_axis(&manifold, sep, 12, normal);

    // Test axis a.z x b.y
    a_radius = a->extents[0] * abs_r[1][1] + a->extents[1] * abs_r[0][1];
    b_radius = b->extents[0] * abs_r[2][2] + b->extents[2] * abs_r[2][0];
    a_to_b = abs(t[1] * r[0][1] - t[0] * r[1][1]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    // normal = v3(-r[1][1], r[0][1], 0.0f);
    normal = normalize(cross(a_axes[2], b_axes[1]));
    update_manifold_for_edge_axis(&manifold, sep, 13, normal);

    // Test axis a.z x b.z
    a_radius = a->extents[0] * abs_r[1][2] + a->extents[1] * abs_r[0][2];
    b_radius = b->extents[0] * abs_r[2][1] + b->extents[1] * abs_r[2][0];
    a_to_b = abs(t[1] * r[0][2] - t[0] * r[1][2]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    // normal = v3(-r[1][2], r[0][2], 0.0f);
    normal = normalize(cross(a_axes[2], b_axes[2]));
    update_manifold_for_edge_axis(&manifold, sep, 14, normal);
  }

  // Correct normal direction
  if (dot(manifold.normal, t_translation) < 0.0f) {
    manifold.normal = -manifold.normal;
  }

  if (manifold.axis < 6) {
    // This is where face-something will go.
  } else {
    uint32 edge_axis = manifold.axis - 6;
    uint32 a_axis = edge_axis / 3;
    uint32 b_axis = edge_axis % 3;

    v3 a_edge_point = a->extents;
    v3 b_edge_point = b->extents;
    for_range_named (i, 0, 3) {
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

    // Change basis into world space
    m3 a_cob = m3(a_axes[0], a_axes[1], a_axes[2]);
    m3 b_cob = m3(b_axes[0], b_axes[1], b_axes[2]);
    a_edge_point = a_cob * a_edge_point + a->center;
    b_edge_point = b_cob * b_edge_point + b->center;

    v3 contact_point = get_edge_contact_point(
      a_edge_point,
      a_axes[a_axis],
      a->extents[a_axis],
      b_edge_point,
      b_axes[b_axis],
      b->extents[b_axis],
      face_axis_with_max_separation > 2
    );

    DebugDraw::draw_point(
      g_dds,
      a_edge_point,
      0.1f,
      v4(1.0f, 0.0f, 1.0f, 1.0f)
    );
    DebugDraw::draw_point(
      g_dds,
      b_edge_point,
      0.1f,
      v4(1.0f, 0.0f, 1.0f, 1.0f)
    );
    DebugDraw::draw_point(
      g_dds,
      contact_point,
      0.1f,
      v4(1.0f, 0.0f, 1.0f, 1.0f)
    );
  }

  // Since no separating axis is found, the OBBs must be intersecting
  // NOTE: manifold.collidee should be filled in by the caller.
  manifold.did_collide = true;
  return manifold;
}


CollisionManifold Physics::find_physics_component_collision(
  PhysicsComponent *self_physics,
  SpatialComponent *self_spatial,
  PhysicsComponentSet *physics_component_set,
  SpatialComponentSet *spatial_component_set
) {
  for_each (candidate_physics, physics_component_set->components) {
    if (!Entities::is_physics_component_valid(candidate_physics)) {
      continue;
    }
    SpatialComponent *candidate_spatial =
      spatial_component_set->components[candidate_physics->entity_handle];
    if (!candidate_spatial) {
      log_error("Could not get SpatialComponent for candidate");
      return CollisionManifold{};
    }

    if (self_physics == candidate_physics) {
      continue;
    }

    CollisionManifold manifold = Physics::intersect_obb_obb(
      &self_physics->transformed_obb,
      &candidate_physics->transformed_obb,
      self_spatial,
      candidate_spatial
    );
    if (manifold.did_collide) {
      manifold.collidee = candidate_physics;
      return manifold;
    }
  }

  return CollisionManifold{};
}
