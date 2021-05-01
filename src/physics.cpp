Obb Physics::transform_obb(Obb obb, SpatialComponent *spatial) {
  glm::mat3 rotation = glm::toMat3(glm::normalize(spatial->rotation));
  obb.center = spatial->position + (rotation * (spatial->scale * obb.center));
  obb.axes[0] = glm::normalize(rotation * obb.axes[0]);
  obb.axes[1] = glm::normalize(rotation * obb.axes[1]);
  obb.extents *= spatial->scale;
  return obb;
}


bool32 Physics::intersect_obb_obb(Obb *obb1, Obb *obb2) {
  // Christer Ericson, Real-Time Collision Detection, 4.4
  real32 ra, rb;
  glm::mat3 r, abs_r;

  glm::vec3 obb1_axes[3] = {
    obb1->axes[0], obb1->axes[1], glm::cross(obb1->axes[0], obb1->axes[1])
  };
  glm::vec3 obb2_axes[3] = {
    obb2->axes[0], obb2->axes[1], glm::cross(obb2->axes[0], obb2->axes[1])
  };

  // Compute rotation matrix expressing obb2 in obb1's coordinate frame
  for_range_named (i, 0, 3) {
    for_range_named (j, 0, 3) {
      r[i][j] = glm::dot(obb1_axes[i], obb2_axes[j]);
    }
  }

  // Compute translation vector t
  glm::vec3 t = obb2->center - obb1->center;

  // Bring translation into obb1's coordinate frame
  t = glm::vec3(
    glm::dot(t, obb1_axes[0]),
    glm::dot(t, obb1_axes[1]),
    glm::dot(t, obb1_axes[2])
  );

  // Compute common subexpressions. Add in an epsilon term to counteract
  // arithmetic errors when two edges are parallel and their cross product
  // is (near) null.
  for_range_named (i, 0, 3) {
    for_range_named (j, 0, 3) {
      abs_r[i][j] = glm::abs(r[i][j]) + FLT_EPSILON;
    }
  }

  // Test axes L = A0, L = A1, L = A2 (axes of obb1)
  for_range_named (i, 0, 3) {
    ra = obb1->extents[i];
    rb = obb2->extents[0] * abs_r[i][0] +
      obb2->extents[1] * abs_r[i][1] +
      obb2->extents[2] * abs_r[i][2];
    if (glm::abs(t[i]) > ra + rb) {
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
    if (glm::abs(abs_argument) > ra + rb) {
      return false;
    }
  }

  // Test axis L = A0 x B0
  ra = obb1->extents[1] * abs_r[2][0] + obb1->extents[2] * abs_r[1][0];
  rb = obb2->extents[1] * abs_r[0][2] + obb2->extents[2] * abs_r[0][1];
  if (glm::abs(t[2] * r[1][0] - t[1] * r[2][0]) > ra + rb) {
    return false;
  }

  // Test axis L = A0 x B1
  ra = obb1->extents[1] * abs_r[2][1] + obb1->extents[2] * abs_r[1][1];
  rb = obb2->extents[0] * abs_r[0][2] + obb2->extents[2] * abs_r[0][0];
  if (glm::abs(t[2] * r[1][1] - t[1] * r[2][1]) > ra + rb) {
    return false;
  }

  // Test axis L = A0 x B2
  ra = obb1->extents[1] * abs_r[2][2] + obb1->extents[2] * abs_r[1][2];
  rb = obb2->extents[0] * abs_r[0][1] + obb2->extents[1] * abs_r[0][0];
  if (glm::abs(t[2] * r[1][2] - t[1] * r[2][2]) > ra + rb) {
    return false;
  }

  // Test axis L = A1 x B0
  ra = obb1->extents[0] * abs_r[2][0] + obb1->extents[2] * abs_r[0][0];
  rb = obb2->extents[1] * abs_r[1][2] + obb2->extents[2] * abs_r[1][1];
  if (glm::abs(t[0] * r[2][0] - t[2] * r[0][0]) > ra + rb) {
    return false;
  }

  // Test axis L = A1 x B1
  ra = obb1->extents[0] * abs_r[2][1] + obb1->extents[2] * abs_r[0][1];
  rb = obb2->extents[0] * abs_r[1][2] + obb2->extents[2] * abs_r[1][0];
  if (glm::abs(t[0] * r[2][1] - t[2] * r[0][1]) > ra + rb) {
    return false;
  }

  // Test axis L = A1 x B2
  ra = obb1->extents[0] * abs_r[2][2] + obb1->extents[2] * abs_r[0][2];
  rb = obb2->extents[0] * abs_r[1][1] + obb2->extents[1] * abs_r[1][0];
  if (glm::abs(t[0] * r[2][2] - t[2] * r[0][2]) > ra + rb) {
    return false;
  }

  // Test axis L = A2 x B0
  ra = obb1->extents[0] * abs_r[1][0] + obb1->extents[1] * abs_r[0][0];
  rb = obb2->extents[1] * abs_r[2][2] + obb2->extents[2] * abs_r[2][1];
  if (glm::abs(t[1] * r[0][0] - t[0] * r[1][0]) > ra + rb) {
    return false;
  }

  // Test axis L = A2 x B1
  ra = obb1->extents[0] * abs_r[1][1] + obb1->extents[1] * abs_r[0][1];
  rb = obb2->extents[0] * abs_r[2][2] + obb2->extents[2] * abs_r[2][0];
  if (glm::abs(t[1] * r[0][1] - t[0] * r[1][1]) > ra + rb) {
    return false;
  }

  // Test axis L = A2 x B2
  ra = obb1->extents[0] * abs_r[1][2] + obb1->extents[1] * abs_r[0][2];
  rb = obb2->extents[0] * abs_r[2][1] + obb2->extents[1] * abs_r[2][0];
  if (glm::abs(t[1] * r[0][2] - t[0] * r[1][2]) > ra + rb) {
    return false;
  }

  // Since no separating axis is found, the OBBs must be intersecting
  return true;
}
