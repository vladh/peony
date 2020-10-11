#define SHOULD_LINEARISE_ALBEDO true

uniform sampler2D g_position_texture;
uniform sampler2D g_normal_texture;
uniform sampler2D g_albedo_texture;
uniform sampler2D g_pbr_texture;

uniform int n_depth_textures;
uniform samplerCube depth_textures[MAX_N_SHADOW_FRAMEBUFFERS];

in BLOCK {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;


// TODO: MOVE THIS TO COMMON. //////////////////////////
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
/////////////////////////////////////////////////////////

void main() {
  vec3 world_position = texture(g_position_texture, fs_in.tex_coords).rgb;
  vec3 normal = texture(g_normal_texture, fs_in.tex_coords).rgb;

  // Skip pixels with no normal. These are the background pixels, so we
  // let the background color shine through.
  if (normal == vec3(0.0, 0.0, 0.0)) {
    discard;
  }

  // Albedo is generally in sRGB space, so we convert it to linear space
  vec3 albedo;
  if (SHOULD_LINEARISE_ALBEDO) {
    albedo = pow(texture(g_albedo_texture, fs_in.tex_coords).rgb, vec3(2.2));
  } else {
    albedo = texture(g_albedo_texture, fs_in.tex_coords).rgb;
  }

  vec4 pbr_texture = texture(g_pbr_texture, fs_in.tex_coords);
  float metallic = pbr_texture.r;
  float roughness = pbr_texture.g;
  float ao = pbr_texture.b;

  vec3 N = normal;
  vec3 V = normalize(camera_position - world_position);
  float n_dot_v = max(dot(N, V), M_EPSILON);
  vec3 F0 = mix(vec3(0.04), albedo, metallic);
  vec3 Lo = vec3(0.0);

  for (int idx_light = 0; idx_light < n_lights; idx_light++) {
    float shadow = 0;

    if (USE_SHADOWS && n_depth_textures >= n_lights) {
      RUN_CALCULATE_SHADOWS_ALL(world_position, idx_light);
    }

    Lo += compute_sphere_light(
      world_position, vec3(light_position[idx_light]), vec3(light_color[idx_light]),
      light_attenuation[idx_light],
      albedo, metallic, roughness,
      N, V,
      n_dot_v, F0
    ) * (1.0 - shadow);
  }

  // TODO: Add better ambient term.
  vec3 ambient = vec3(0.03) * albedo * ao;
  vec3 color = ambient + Lo;

  color = add_tone_mapping(color);
  color = correct_gamma(color);

  frag_color = vec4(color, 1.0);
}
