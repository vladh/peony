#version 330 core

#define MAX_N_LIGHTS 32
#define MAX_N_SHADOW_FRAMEBUFFERS MAX_N_LIGHTS
#define MAX_N_TEXTURES 32
#define USE_SHADOWS true

// NOTE: We need this hack because GLSL doesn't allow us to index samplerCubes
// by non-constant indices, so we can't do depth_textures[idx_light].
// Hopefully this will go away in the future (deferred lighting?).

#define RUN_CALCULATE_SHADOWS(frag_position, idx_light, idx_texture) { \
  if (idx_texture < n_depth_textures && idx_light == idx_texture) { \
    shadow += calculate_shadows(frag_position, idx_light, depth_textures[idx_texture]); \
  } \
}

#define RUN_CALCULATE_SHADOWS_32(frag_position, idx_light) { \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 0); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 1); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 2); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 3); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 4); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 5); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 6); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 7); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 8); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 9); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 10); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 11); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 12); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 13); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 14); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 15); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 16); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 17); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 18); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 19); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 20); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 21); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 22); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 23); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 24); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 25); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 26); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 27); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 28); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 29); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 30); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 31); \
}

#define GAMMA 2.2

struct Light {
  vec3 position;
  float pad_3;
  vec3 direction;
  float pad_7;
  vec3 ambient;
  float pad_11;
  vec3 diffuse;
  float pad_15;
  vec3 specular;
  float pad_19;
  float attenuation_constant;
  float attenuation_linear;
  float attenuation_quadratic;
  float pad_23;
};

layout (std140) uniform shader_common {
  mat4 view;
  mat4 projection;
  mat4 shadow_transforms[6];
  vec3 camera_position;
  float pad_15;
  float t;
  float far_clip_dist;
  int n_lights;
  float pad_19;
  Light lights[MAX_N_LIGHTS];
};

uniform int n_diffuse_textures;
uniform sampler2D diffuse_textures[MAX_N_TEXTURES];
uniform int n_specular_textures;
uniform sampler2D specular_textures[MAX_N_TEXTURES];
uniform int n_depth_textures;
uniform samplerCube depth_textures[MAX_N_SHADOW_FRAMEBUFFERS];

uniform float exposure;

in VS_OUT {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;

vec3 invert(vec3 rgb) {
  return 1 - rgb;
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

#if 0
void main() {
  vec3 rgb = texture(diffuse_textures[0], fs_in.tex_coords).rgb;

  rgb = add_tone_mapping(rgb);
  rgb = correct_gamma(rgb);
  /* rgb = invert(rgb); */

  frag_color = vec4(rgb, 1.0f);
}
#endif

vec3 grid_sampling_offsets[20] = vec3[] (
  vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
  vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
  vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
  vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
  vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float calculate_shadows(vec3 frag_position, int idx_light, samplerCube depth_texture) {
  vec3 frag_to_light = frag_position - lights[idx_light].position;
  float current_depth = length(frag_to_light);

  float shadow = 0.0f;
  float bias = 0.30f;
  int n_samples = 20;

  float view_distance = length(camera_position - frag_position);
  float sample_radius = (1.0f + (view_distance / far_clip_dist)) / 25.0f;

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

void main() {
  // retrieve data from G-buffer
  vec3 frag_position = texture(diffuse_textures[0], fs_in.tex_coords).rgb;
  vec3 normal = texture(diffuse_textures[1], fs_in.tex_coords).rgb;
  vec3 diffuse_texture_0 = texture(diffuse_textures[2], fs_in.tex_coords).rgb;
  float specular_texture_0 = texture(diffuse_textures[2], fs_in.tex_coords).a;

  if (normal == vec3(0.0f, 0.0f, 0.0f)) {
    discard;
  }

  vec3 unit_normal = normalize(normal);
  vec3 lighting = vec3(0.0f, 0.0f, 0.0f);

  for (int idx_light = 0; idx_light < n_lights; idx_light++) {
    Light light = lights[idx_light];

    // Ambient
    vec3 ambient = light.ambient;

    // Diffuse
    vec3 light_dir = normalize(light.position - frag_position);
    float diffuse_intensity = max(dot(unit_normal, light_dir), 0.0f);
    vec3 diffuse = light.diffuse * diffuse_intensity;

    // Specular
    vec3 view_dir = normalize(camera_position - frag_position);
    vec3 reflect_dir = reflect(-light_dir, unit_normal);
    float specular_intensity = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = light.specular * specular_intensity;

    // Attenuation
    float distance = length(light.position - frag_position);
    float attenuation = 1.0 / (
      light.attenuation_constant +
      light.attenuation_linear * distance +
      light.attenuation_quadratic * (distance * distance)
    );
    diffuse *= attenuation;
    specular *= attenuation;

    if (n_depth_textures >= n_lights && USE_SHADOWS) {
      float shadow = 0;
      RUN_CALCULATE_SHADOWS_32(frag_position, idx_light);
      lighting += (ambient + ((1.0f - shadow) * (diffuse + specular))) * diffuse_texture_0;
    } else {
      lighting += (ambient + diffuse + specular) * diffuse_texture_0;
    }
  }

  lighting = add_tone_mapping(lighting);
  lighting = correct_gamma(lighting);

  frag_color = vec4(lighting, 1.0f);
}
