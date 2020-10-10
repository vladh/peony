// NOTE: We need this hack because GLSL doesn't allow us to index samplerCubes
// by non-constant indices, so we can't do depth_textures[idx_light].
// Hopefully this will go away in the future (deferred lighting?).

#define RUN_CALCULATE_SHADOWS(world_position, idx_light, idx_texture) { \
  if (idx_texture < n_depth_textures && idx_light == idx_texture) { \
    shadow += calculate_shadows(world_position, idx_light, depth_textures[idx_texture]); \
  } \
}

#define RUN_CALCULATE_SHADOWS_ALL(world_position, idx_light) { \
  RUN_CALCULATE_SHADOWS(world_position, idx_light, 0); \
  RUN_CALCULATE_SHADOWS(world_position, idx_light, 1); \
  RUN_CALCULATE_SHADOWS(world_position, idx_light, 2); \
  RUN_CALCULATE_SHADOWS(world_position, idx_light, 3); \
  RUN_CALCULATE_SHADOWS(world_position, idx_light, 4); \
  RUN_CALCULATE_SHADOWS(world_position, idx_light, 5); \
  RUN_CALCULATE_SHADOWS(world_position, idx_light, 6); \
  RUN_CALCULATE_SHADOWS(world_position, idx_light, 7); \
}

// A simplified way to get our tangent-normals to world-space from LearnOpenGL.
// Don't really understand how this works!
// We probably want to convert this to the regular way of calculating them
// in the future, but since we're using both PBR and deferred lighting,
// it would be a bit troublesome to integrate into the code.
vec3 get_normal_from_map(
  vec4 normal_map_sample,
  vec3 world_position,
  vec3 normal,
  vec2 tex_coords
) {
  // TODO: Swizzle this differently.
  vec3 tangent_normal_rgb = normal_map_sample.xyz * 2.0 - 1.0;
  vec3 tangent_normal = vec3(
    tangent_normal_rgb.b, tangent_normal_rgb.g, tangent_normal_rgb.r
  );

  vec3 Q1 = dFdx(world_position);
  vec3 Q2 = dFdy(world_position);
  vec2 st1 = dFdx(tex_coords);
  vec2 st2 = dFdy(tex_coords);

  vec3 N = normalize(normal);
  vec3 T = normalize((Q1 * st2.t) - (Q2 * st1.t));
  vec3 B = -normalize(cross(N, T));
  mat3 TBN = mat3(T, B, N);

  return normalize(TBN * tangent_normal);
}
