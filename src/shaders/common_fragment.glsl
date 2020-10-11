#define GAMMA 2.2
#define USE_SHADOWS true

vec3 grid_sampling_offsets[20] = vec3[] (
  vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
  vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
  vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
  vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
  vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);


float calculate_shadows(vec3 world_position, int idx_light, samplerCube depth_texture) {
  vec3 frag_to_light = world_position - vec3(light_position[idx_light]);
  float current_depth = length(frag_to_light);

  float shadow = 0.0;
  float bias = 0.15;
  int n_samples = 10;

  float view_distance = length(camera_position - world_position);
  float sample_radius = (1.0 + (view_distance / far_clip_dist)) / 25.0;

  for (int i = 0; i < n_samples; i++) {
    float closest_depth = texture(
      depth_texture,
      frag_to_light + grid_sampling_offsets[i] * sample_radius
    ).r * far_clip_dist;

    if (current_depth - bias > closest_depth) {
      shadow += 1.0;
    }
  }
  shadow /= float(n_samples);

  return shadow;
}


// NOTE: We need this hack because GLSL doesn't allow us to index samplerCubes
// by non-constant indices, so we can't do depth_textures[idx_light].
// We could fix this by using cube_map_arrays, but that would require
// switching to OpenGL 4.1, since they are not supported on macOS
// in 3.3 even with an extension.

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


vec3 correct_gamma(vec3 rgb) {
  return pow(rgb, vec3(1.0 / GAMMA));
}


vec3 add_tone_mapping(vec3 rgb) {
  // Reinhard tone mapping
  /* return rgb / (rgb + vec3(1.0)); */
  // Exposure tone mapping
  return vec3(1.0) - exp(-rgb * exposure);
}


vec3 fresnel_schlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}


float distribution_ggx(float n_dot_h, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float n_dot_h_2 = n_dot_h * n_dot_h;

  float num = a2;
  float denom = (n_dot_h_2 * (a2 - 1.0) + 1.0);
  denom = M_PI * denom * denom;

  return num / denom;
}


float geometry_schlick_ggx(float n_dot_v, float roughness) {
  float k = pow(roughness + 1.0, 2) / 8.0;

  float num = n_dot_v;
  float denom = n_dot_v * (1.0 - k) + k;

  return num / denom;
}


float geometry_smith(float n_dot_v, float n_dot_l, float roughness) {
  float ggx1 = geometry_schlick_ggx(n_dot_v, roughness);
  float ggx2 = geometry_schlick_ggx(n_dot_l, roughness);

  return ggx1 * ggx2;
}


float geometry_smith_ggx_schlick_alternate(
  float n_dot_v, float n_dot_l, float roughness
) {
  float rough2 = roughness * roughness;
  float lambda_v = n_dot_l  * sqrt((-n_dot_v * rough2 + n_dot_v) * n_dot_v + rough2);
  float lambda_l = n_dot_v  * sqrt((-n_dot_l * rough2 + n_dot_l) * n_dot_l + rough2);

  return 0.5 / (lambda_v + lambda_l);
}


float neumann_visibility(float n_dot_v, float n_dot_l) {
  return n_dot_l * n_dot_v / max(1e-7, max(n_dot_l, n_dot_v));
}


vec3 compute_sphere_light(
  vec3 world_position, vec3 light_position, vec3 light_color,
  vec4 light_attenuation,
  vec3 albedo, float metallic, float roughness,
  vec3 N, vec3 V,
  float n_dot_v, vec3 F0
) {
  vec3 L = normalize(light_position - world_position);
  vec3 H = normalize(V + L);
  float n_dot_l = max(dot(N, L), M_EPSILON);
  float h_dot_v = max(dot(H, V), M_EPSILON);
  float n_dot_h = max(dot(N, H), M_EPSILON);

  float distance = length(light_position - world_position);
  float attenuation = 1.0 / (
    light_attenuation[0] +
    light_attenuation[1] * distance +
    light_attenuation[2] * (distance * distance)
  );
  vec3 radiance = light_color * attenuation;

  float NDF = distribution_ggx(n_dot_h, roughness);
  float G = geometry_smith(n_dot_v, n_dot_l, roughness);
  vec3 F = fresnel_schlick(h_dot_v, F0);

  vec3 numerator = NDF * G * F;
  float denominator = 4.0 * n_dot_v * n_dot_l;
  vec3 specular = numerator / max(denominator, M_EPSILON);

  vec3 kS = F;
  vec3 kD = vec3(1.0) - kS;
  kD *= 1.0 - metallic;

  return (kD * albedo / M_PI + specular) * radiance * n_dot_l;
}
