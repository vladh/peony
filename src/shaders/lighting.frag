#define GAMMA 2.2
#define USE_SHADOWS true
#define SHOULD_LINEARISE_ALBEDO true

vec3 grid_sampling_offsets[20] = vec3[] (
  vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
  vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
  vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
  vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
  vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

uniform sampler2D g_position_texture;
uniform sampler2D g_normal_texture;
uniform sampler2D g_albedo_texture;
uniform sampler2D g_pbr_texture;

uniform int n_depth_textures;
uniform samplerCube depth_textures[MAX_N_SHADOW_FRAMEBUFFERS];

in VS_OUT {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;

vec3 correct_gamma(vec3 rgb) {
  return pow(rgb, vec3(1.0 / GAMMA));
}

vec3 add_tone_mapping(vec3 rgb) {
  // Reinhard tone mapping
  /* return rgb / (rgb + vec3(1.0)); */
  // Exposure tone mapping
  return vec3(1.0) - exp(-rgb * exposure);
}

float calculate_shadows(vec3 frag_position, int idx_light, samplerCube depth_texture) {
  vec3 frag_to_light = frag_position - vec3(light_position[idx_light]);
  float current_depth = length(frag_to_light);

  float shadow = 0.0;
  float bias = 0.20;
  int n_samples = 20;

  float view_distance = length(camera_position - frag_position);
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

vec3 fresnel_schlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distribution_ggx(vec3 N, vec3 H, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float num = a2;
  float denom = (NdotH2 * (a2 - 1.0) + 1.0);
  denom = PI * denom * denom;

  return num / denom;
}

float geometry_schlick_ggx(float nv, float roughness) {
  float k = pow(roughness + 1.0, 2) / 8.0;

  float num = nv;
  float denom = nv * (1.0 - k) + k;

  return num / denom;
}

float geometry_smith(vec3 N, vec3 V, vec3 L, float roughness) {
  float nv = max(dot(N, V), 0.0);
  float n_dot_l = max(dot(N, L), 0.0);
  float ggx1 = geometry_schlick_ggx(nv, roughness);
  float ggx2 = geometry_schlick_ggx(n_dot_l, roughness);

  return ggx1 * ggx2;
}

void main() {
  vec3 frag_position = texture(g_position_texture, fs_in.tex_coords).rgb;
  vec3 normal = texture(g_normal_texture, fs_in.tex_coords).rgb;

  // Skip pixels with no normal. These are the background pixels, hence
  // letting the background color shine through.
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
  vec3 V = normalize(camera_position - frag_position);

  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  vec3 Lo = vec3(0.0);

  for (int idx_light = 0; idx_light < n_lights; idx_light++) {
    vec3 L = normalize(vec3(light_position[idx_light]) - frag_position);
    vec3 H = normalize(V + L);

    float distance = length(vec3(light_position[idx_light]) - frag_position);
    float attenuation = 1.0 / (
      light_attenuation[idx_light][0] +
      light_attenuation[idx_light][1] * distance +
      light_attenuation[idx_light][2] * (distance * distance)
    );
    vec3 radiance = vec3(light_color[idx_light]) * attenuation;

    float NDF = distribution_ggx(N, H, roughness);
    float G = geometry_smith(N, V, L, roughness);
    vec3 F = fresnel_schlick(max(dot(H, V), 0.0), F0);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0);
    vec3 specular = numerator / max(denominator, 0.001);

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float n_dot_l = max(dot(N, L), 0.0);

    float shadow = 0;

    if (USE_SHADOWS && n_depth_textures >= n_lights) {
      RUN_CALCULATE_SHADOWS_ALL(frag_position, idx_light);
    }

    Lo += (kD * albedo / PI + specular) * radiance * n_dot_l * (1.0 - shadow);
  }

  vec3 ambient = vec3(0.03) * albedo * ao;
  vec3 color = ambient + Lo;

  color = add_tone_mapping(color);
  color = correct_gamma(color);

  frag_color = vec4(color, 1.0);
}
