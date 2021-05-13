#define GAMMA 2.2
#define WATER_F0 0.02037
#define N_SHADOW_SAMPLES 20
#define SHOULD_USE_SHADOWS 1

uniform samplerCubeArray shadowmaps_3d;
uniform sampler2DArray shadowmaps_2d;

// NOTE: First directional light is the sun.
vec3 SUN_DIRECTION = normalize(vec3(directional_light_direction[0]));
vec3 SUN_DIRECTION_AT_SUNSET = normalize(vec3(SUN_DIRECTION.x, 0.0, SUN_DIRECTION.z));
float SUN_DOT = dot(SUN_DIRECTION, SUN_DIRECTION_AT_SUNSET);
float SUN_ANGLE = acos(SUN_DOT);
float SUN_SIZE = 0.1;
float SUN_BRIGHTNESS = 50.0;
float SUNSET_FACTOR = clamp(pow(SUN_DOT, 4), 0.0, 1.0);
float SUNSET_LIGHT_FACTOR = clamp((1.0 - SUNSET_FACTOR) + 0.1, 0.0, 1.0);

vec3 SKY_ALBEDO = mix(
  vec3(0.56, 0.77, 0.95),
  vec3(0.26, 0.17, 0.31),
  SUNSET_FACTOR
);
vec3 GROUND_ALBEDO = mix(
  vec3(1.0, 1.0, 1.0),
  vec3(0.98, 0.40, 0.23),
  SUNSET_FACTOR
);
vec3 SKY_REFLECTION_ALBEDO = mix(
  SKY_ALBEDO,
  GROUND_ALBEDO,
  SUNSET_FACTOR
);
vec3 SUN_ALBEDO = mix(
  vec3(1.00, 1.00, 1.00),
  vec3(0.96, 0.86, 0.50),
  SUNSET_FACTOR
) * SUN_BRIGHTNESS;

vec3 BLOOM_BRIGHTNESS_THRESHOLD = vec3(0.2126, 0.7152, 0.0722);

vec3 FOG_ALBEDO = vec3(0.4, 0.4, 0.8);

vec3 SHADOW_GRID_SAMPLING_OFFSETS[20] = vec3[] (
  vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
  vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
  vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
  vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
  vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

vec2 TEXTURE_SAMPLING_OFFSETS[9] = vec2[] (
  vec2( 0.0,  0.0),
  vec2(-1.0,  0.0), vec2(1.0,  0.0),
  vec2( 0.0, -1.0), vec2(0.0,  1.0),
  vec2(-1.0, -1.0), vec2(1.0, -1.0),
  vec2(-1.0,  1.0), vec2(1.0,  1.0)
);


vec3 get_sky_color(float angle) {
  return mix(
    GROUND_ALBEDO,
    SKY_ALBEDO,
    // [-90, 90] → [0, 180] → [0, 1]
    (angle + 90.0) / 180.0
  );
}


float calculate_point_shadows(
  vec3 world_position,
  vec3 N,
  vec4 current_light_position,
  int idx_light
) {
#if !SHOULD_USE_SHADOWS
  return 0.0;
#endif

  float shadow = 0.0;
  float bias = 0.20 / shadow_far_clip_dist;

  vec3 light_to_frag = world_position - vec3(current_light_position);
  float view_distance = length(camera_position - world_position);

  // TODO: Improve this.
  float sample_radius = (1.0 + (view_distance / shadow_far_clip_dist)) / 25.0;
  float current_depth = length(light_to_frag) / shadow_far_clip_dist;

  for (int i = 0; i < N_SHADOW_SAMPLES; i++) {
    vec4 sample_p = vec4(
      light_to_frag + SHADOW_GRID_SAMPLING_OFFSETS[i] * sample_radius, idx_light
    );
    float closest_depth = texture(shadowmaps_3d, sample_p).r;
    if (current_depth - bias > closest_depth) {
      shadow += 1.0;
    }
  }

  shadow /= float(N_SHADOW_SAMPLES);

  return shadow;
}


float calculate_directional_shadows(
  vec3 world_position,
  vec3 N,
  vec4 current_light_position,
  vec4 current_light_direction,
  int idx_light
) {
#if !SHOULD_USE_SHADOWS
  return 0.0;
#endif

  float shadow = 0.0;
  float bias = 0.20 / shadow_far_clip_dist;

  vec3 light_to_frag = world_position - vec3(current_light_position);
  float view_distance = length(camera_position - world_position);

  // TODO: Improve this.
  float sample_radius = (1.0 + (view_distance / shadow_far_clip_dist)) / 1250.0;
  float depth_sign = sign(dot(light_to_frag, vec3(current_light_direction)));
  float current_depth = length(light_to_frag) * depth_sign / shadow_far_clip_dist;

  vec3 light_space_position = vec3(
    shadowmap_2d_transforms[idx_light] * vec4(world_position, 1.0)
  ) * 0.5 + 0.5;

  for (int i = 0; i < N_SHADOW_SAMPLES; i++) {
    vec3 sample_p = vec3(
      light_space_position.xy + SHADOW_GRID_SAMPLING_OFFSETS[i].xy * sample_radius, idx_light
    );
    float closest_depth = texture(shadowmaps_2d, sample_p).r;
    if (current_depth - bias > closest_depth) {
      shadow += 1.0;
    }
  }

  shadow /= float(N_SHADOW_SAMPLES);

  return shadow;
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


vec3 desaturate(vec3 color, float factor) {
  // https://github.com/jamieowen/glsl-blend/blob/master/_temp/conversion/desaturate.glsl
  vec3 mask = vec3(0.3, 0.59, 0.11);
  vec3 gray = vec3(dot(mask, color));
  return mix(color, gray, factor);
}


vec3 linearize_albedo(vec3 albedo) {
  return pow(albedo, vec3(2.2));
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


float fresnel_schlick(float cosTheta, float F0) {
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


vec3 compute_directional_light(
  vec3 world_position,
  vec3 light_color,
  vec4 light_direction,
  vec3 albedo, float metallic, float roughness,
  vec3 N, vec3 V,
  float n_dot_v,
  vec3 F0
) {
  vec3 L = normalize(vec3(-light_direction));
  vec3 H = normalize(V + L);
  float n_dot_l = max(dot(N, L), M_EPSILON);
  float h_dot_v = max(dot(H, V), M_EPSILON);
  float n_dot_h = max(dot(N, H), M_EPSILON);

  vec3 radiance = light_color;

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


vec3 compute_point_light(
  vec3 world_position,
  vec3 curr_light_position,
  vec3 light_color,
  vec4 light_attenuation,
  vec3 albedo, float metallic, float roughness,
  vec3 N, vec3 V,
  float n_dot_v,
  vec3 F0
) {
  vec3 L = normalize(curr_light_position - world_position);
  vec3 H = normalize(V + L);
  float n_dot_l = max(dot(N, L), M_EPSILON);
  float h_dot_v = max(dot(H, V), M_EPSILON);
  float n_dot_h = max(dot(N, H), M_EPSILON);

  float distance = length(curr_light_position - world_position);
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

vec3 compute_pbr_light(
  vec3 albedo, float metallic, float roughness, float ao,
  vec3 world_position, vec3 normal
) {
  vec3 N = normal;
  vec3 V = normalize(camera_position - world_position);
  float n_dot_v = max(dot(N, V), M_EPSILON);
  vec3 F0 = mix(vec3(0.04), albedo, metallic);
  vec3 Lo = vec3(0.0);

  for (int idx_light = 0; idx_light < n_point_lights; idx_light++) {
    float shadow = calculate_point_shadows(
      world_position,
      N,
      point_light_position[idx_light],
      idx_light
    );

    Lo += compute_point_light(
      world_position,
      vec3(point_light_position[idx_light]),
      vec3(point_light_color[idx_light]),
      point_light_attenuation[idx_light],
      albedo, metallic, roughness,
      N, V,
      n_dot_v,
      F0
    ) * (1.0 - shadow);
  }

  for (int idx_light = 0; idx_light < n_directional_lights; idx_light++) {
    float shadow = calculate_directional_shadows(
      world_position,
      N,
      directional_light_position[idx_light],
      directional_light_direction[idx_light],
      idx_light
    );

    Lo += compute_directional_light(
      world_position, vec3(directional_light_color[idx_light]),
      directional_light_direction[idx_light],
      albedo, metallic, roughness,
      N, V,
      n_dot_v,
      F0
    ) * (1.0 - shadow);
  }

  // TODO: Add better ambient term.
  vec3 ambient = vec3(0.20) * albedo * ao;
  vec3 color = ambient + Lo;

  return color;
}
