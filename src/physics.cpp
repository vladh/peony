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


bool32 Physics::intersect_obb_obb(Obb *obb1, Obb *obb2) {
  // Christer Ericson, Real-Time Collision Detection, 4.4
  // This is the separating axis test (sometimes abbreviated SAT).
  real32 ra, rb;
  m3 r, abs_r;

  v3 obb1_axes[3] = {
    obb1->x_axis, obb1->y_axis, cross(obb1->x_axis, obb1->y_axis)
  };
  v3 obb2_axes[3] = {
    obb2->x_axis, obb2->y_axis, cross(obb2->x_axis, obb2->y_axis)
  };

  // Compute rotation matrix expressing obb2 in obb1's coordinate frame
  for_range_named (i, 0, 3) {
    for_range_named (j, 0, 3) {
      r[i][j] = dot(obb1_axes[i], obb2_axes[j]);
    }
  }

  // Compute translation vector t
  v3 t = obb2->center - obb1->center;

  // Bring translation into obb1's coordinate frame
  t = v3(
    dot(t, obb1_axes[0]),
    dot(t, obb1_axes[1]),
    dot(t, obb1_axes[2])
  );

  // Compute common subexpressions. Add in an epsilon term to counteract
  // arithmetic errors when two edges are parallel and their cross product
  // is (near) null.
  for_range_named (i, 0, 3) {
    for_range_named (j, 0, 3) {
      abs_r[i][j] = abs(r[i][j]) + FLT_EPSILON;
    }
  }

  // Test axes L = A0, L = A1, L = A2 (axes of obb1)
  for_range_named (i, 0, 3) {
    ra = obb1->extents[i];
    rb = obb2->extents[0] * abs_r[i][0] +
      obb2->extents[1] * abs_r[i][1] +
      obb2->extents[2] * abs_r[i][2];
    if (abs(t[i]) > ra + rb) {
      return false;
    }
  }

  // Test axes L = B0, L = B1, L = B2 (axes of obb2)
  for_range_named (i, 0, 3) {
    ra = obb1->extents[0] * abs_r[0][i] +
      obb1->extents[1] * abs_r[1][i] +
      obb1->extents[2] * abs_r[2][i];
    rb = obb2->extents[i];
    real32 abs_argument = t[0] * r[0][i] +
      t[1] * r[1][i] +
      t[2] * r[2][i];
    if (abs(abs_argument) > ra + rb) {
      return false;
    }
  }

  // Test axis L = A0 x B0
  ra = obb1->extents[1] * abs_r[2][0] + obb1->extents[2] * abs_r[1][0];
  rb = obb2->extents[1] * abs_r[0][2] + obb2->extents[2] * abs_r[0][1];
  if (abs(t[2] * r[1][0] - t[1] * r[2][0]) > ra + rb) {
    return false;
  }

  // Test axis L = A0 x B1
  ra = obb1->extents[1] * abs_r[2][1] + obb1->extents[2] * abs_r[1][1];
  rb = obb2->extents[0] * abs_r[0][2] + obb2->extents[2] * abs_r[0][0];
  if (abs(t[2] * r[1][1] - t[1] * r[2][1]) > ra + rb) {
    return false;
  }

  // Test axis L = A0 x B2
  ra = obb1->extents[1] * abs_r[2][2] + obb1->extents[2] * abs_r[1][2];
  rb = obb2->extents[0] * abs_r[0][1] + obb2->extents[1] * abs_r[0][0];
  if (abs(t[2] * r[1][2] - t[1] * r[2][2]) > ra + rb) {
    return false;
  }

  // Test axis L = A1 x B0
  ra = obb1->extents[0] * abs_r[2][0] + obb1->extents[2] * abs_r[0][0];
  rb = obb2->extents[1] * abs_r[1][2] + obb2->extents[2] * abs_r[1][1];
  if (abs(t[0] * r[2][0] - t[2] * r[0][0]) > ra + rb) {
    return false;
  }

  // Test axis L = A1 x B1
  ra = obb1->extents[0] * abs_r[2][1] + obb1->extents[2] * abs_r[0][1];
  rb = obb2->extents[0] * abs_r[1][2] + obb2->extents[2] * abs_r[1][0];
  if (abs(t[0] * r[2][1] - t[2] * r[0][1]) > ra + rb) {
    return false;
  }

  // Test axis L = A1 x B2
  ra = obb1->extents[0] * abs_r[2][2] + obb1->extents[2] * abs_r[0][2];
  rb = obb2->extents[0] * abs_r[1][1] + obb2->extents[1] * abs_r[1][0];
  if (abs(t[0] * r[2][2] - t[2] * r[0][2]) > ra + rb) {
    return false;
  }

  // Test axis L = A2 x B0
  ra = obb1->extents[0] * abs_r[1][0] + obb1->extents[1] * abs_r[0][0];
  rb = obb2->extents[1] * abs_r[2][2] + obb2->extents[2] * abs_r[2][1];
  if (abs(t[1] * r[0][0] - t[0] * r[1][0]) > ra + rb) {
    return false;
  }

  // Test axis L = A2 x B1
  ra = obb1->extents[0] * abs_r[1][1] + obb1->extents[1] * abs_r[0][1];
  rb = obb2->extents[0] * abs_r[2][2] + obb2->extents[2] * abs_r[2][0];
  if (abs(t[1] * r[0][1] - t[0] * r[1][1]) > ra + rb) {
    return false;
  }

  // Test axis L = A2 x B2
  ra = obb1->extents[0] * abs_r[1][2] + obb1->extents[1] * abs_r[0][2];
  rb = obb2->extents[0] * abs_r[2][1] + obb2->extents[1] * abs_r[2][0];
  if (abs(t[1] * r[0][2] - t[0] * r[1][2]) > ra + rb) {
    return false;
  }

  // Since no separating axis is found, the OBBs must be intersecting
  return true;
}


PhysicsComponent* Physics::find_physics_component_collision(
  PhysicsComponent *self,
  PhysicsComponentSet *physics_component_set
) {
  for_each (candidate, physics_component_set->components) {
    if (!Entities::is_physics_component_valid(candidate)) {
      continue;
    }

    if (self == candidate) {
      continue;
    }

    if (Physics::intersect_obb_obb(
      &self->transformed_obb, &candidate->transformed_obb
    )) {
      return candidate;
    }
  }

  return nullptr;
}
