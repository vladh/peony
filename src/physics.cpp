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


CollisionManifold Physics::intersect_obb_obb(
  Obb *a,
  Obb *b,
  SpatialComponent *spatial_a,
  SpatialComponent *spatial_b
) {
  // Christer Ericson, Real-Time Collision Detection, 4.4
  // This is the separating axis test (sometimes abbreviated SAT).
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

  // Compute translation vector t
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

  if (do_obbs_share_one_axis) {
    // If the two OBBs share one axis, we can skip checking their cross product
    // axes altogether.
    // NOTE: (@vladh) It's not 100% clear to me why this is.
  } else {
    if (g_use_cross) {
      console_log("Using cross");
    } else {
      console_log("Not using cross");
    }
    // Test axis a.x x b.x
    a_radius = a->extents[1] * abs_r[2][0] + a->extents[2] * abs_r[1][0];
    b_radius = b->extents[1] * abs_r[0][2] + b->extents[2] * abs_r[0][1];
    a_to_b = abs(t[2] * r[1][0] - t[1] * r[2][0]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    if (g_use_cross) {
      normal = cross(a_axes[0], b_axes[0]);
    } else {
      normal = v3(0.0f, -r[2][0], r[1][0]);
    }
    update_manifold_for_edge_axis(&manifold, sep, 6, normal);

    // Test axis a.x x b.y
    a_radius = a->extents[1] * abs_r[2][1] + a->extents[2] * abs_r[1][1];
    b_radius = b->extents[0] * abs_r[0][2] + b->extents[2] * abs_r[0][0];
    a_to_b = abs(t[2] * r[1][1] - t[1] * r[2][1]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    if (g_use_cross) {
      normal = cross(a_axes[0], b_axes[1]);
    } else {
      normal = v3(0.0f, r[2][1], -r[1][1]);
    }
    DebugDraw::draw_line(
      g_dds, a->center, a->center + a_axes[0] * 100.0f, v4(1.0f, 0.0f, 0.0f, 1.0f)
    );
    DebugDraw::draw_line(
      g_dds, b->center, b->center + b_axes[1] * 100.0f, v4(1.0f, 0.0f, 0.0f, 1.0f)
    );
    update_manifold_for_edge_axis(&manifold, sep, 7, normal);

    // Test axis a.x x b.z
    a_radius = a->extents[1] * abs_r[2][2] + a->extents[2] * abs_r[1][2];
    b_radius = b->extents[0] * abs_r[0][1] + b->extents[1] * abs_r[0][0];
    a_to_b = abs(t[2] * r[1][2] - t[1] * r[2][2]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    if (g_use_cross) {
      normal = cross(a_axes[0], b_axes[2]);
    } else {
      normal = v3(0.0f, -r[2][2], r[1][2]);
    }
    update_manifold_for_edge_axis(&manifold, sep, 8, normal);

    // Test axis a.y x b.x
    a_radius = a->extents[0] * abs_r[2][0] + a->extents[2] * abs_r[0][0];
    b_radius = b->extents[1] * abs_r[1][2] + b->extents[2] * abs_r[1][1];
    a_to_b = abs(t[0] * r[2][0] - t[2] * r[0][0]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    if (g_use_cross) {
      normal = cross(a_axes[1], b_axes[0]);
    } else {
      normal = v3(r[2][0], 0.0f, -r[0][0]);
    }
    update_manifold_for_edge_axis(&manifold, sep, 9, normal);

    // Test axis a.y x b.y
    a_radius = a->extents[0] * abs_r[2][1] + a->extents[2] * abs_r[0][1];
    b_radius = b->extents[0] * abs_r[1][2] + b->extents[2] * abs_r[1][0];
    a_to_b = abs(t[0] * r[2][1] - t[2] * r[0][1]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    if (g_use_cross) {
      normal = cross(a_axes[1], b_axes[1]);
    } else {
      normal = v3(r[2][1], 0.0f, -r[0][1]);
    }
    update_manifold_for_edge_axis(&manifold, sep, 10, normal);

    // Test axis a.y x b.z
    a_radius = a->extents[0] * abs_r[2][2] + a->extents[2] * abs_r[0][2];
    b_radius = b->extents[0] * abs_r[1][1] + b->extents[1] * abs_r[1][0];
    a_to_b = abs(t[0] * r[2][2] - t[2] * r[0][2]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    if (g_use_cross) {
      normal = cross(a_axes[1], b_axes[2]);
    } else {
      normal = v3(r[2][2], 0.0f, -r[0][2]);
    }
    update_manifold_for_edge_axis(&manifold, sep, 11, normal);

    // Test axis a.z x b.x
    a_radius = a->extents[0] * abs_r[1][0] + a->extents[1] * abs_r[0][0];
    b_radius = b->extents[1] * abs_r[2][2] + b->extents[2] * abs_r[2][1];
    a_to_b = abs(t[1] * r[0][0] - t[0] * r[1][0]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    if (g_use_cross) {
      normal = cross(a_axes[2], b_axes[0]);
    } else {
      normal = v3(-r[1][0], r[0][0], 0.0f);
    }
    update_manifold_for_edge_axis(&manifold, sep, 12, normal);

    // Test axis a.z x b.y
    a_radius = a->extents[0] * abs_r[1][1] + a->extents[1] * abs_r[0][1];
    b_radius = b->extents[0] * abs_r[2][2] + b->extents[2] * abs_r[2][0];
    a_to_b = abs(t[1] * r[0][1] - t[0] * r[1][1]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    if (g_use_cross) {
      normal = cross(a_axes[2], b_axes[1]);
    } else {
      normal = v3(-r[1][1], r[0][1], 0.0f);
    }
    update_manifold_for_edge_axis(&manifold, sep, 13, normal);

    // Test axis a.z x b.z
    a_radius = a->extents[0] * abs_r[1][2] + a->extents[1] * abs_r[0][2];
    b_radius = b->extents[0] * abs_r[2][1] + b->extents[1] * abs_r[2][0];
    a_to_b = abs(t[1] * r[0][2] - t[0] * r[1][2]);
    sep = a_to_b - (a_radius + b_radius);
    if (sep > 0) { return manifold; }
    if (g_use_cross) {
      normal = cross(a_axes[2], b_axes[2]);
    } else {
      normal = v3(-r[1][2], r[0][2], 0.0f);
    }
    update_manifold_for_edge_axis(&manifold, sep, 14, normal);
  }

  // Correct normal direction
  if (dot(manifold.normal, t_translation) < 0.0f) {
    manifold.normal = -manifold.normal;
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
