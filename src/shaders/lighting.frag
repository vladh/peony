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
