#version 330 core

#define MAX_N_LIGHTS 8
#define MAX_N_SHADOW_FRAMEBUFFERS MAX_N_LIGHTS
#define MAX_N_TEXTURES 8
#define USE_SHADOWS true

// NOTE: We need this hack because GLSL doesn't allow us to index samplerCubes
// by non-constant indices, so we can't do depth_textures[idx_light].
// Hopefully this will go away in the future (deferred lighting?).

#define RUN_CALCULATE_SHADOWS(idx_light, idx_texture) { \
  if (idx_texture < n_depth_textures && idx_light == idx_texture) { \
    shadow += calculate_shadows(fs_in.frag_position, idx_light, depth_textures[idx_texture]); \
  } \
}

#define RUN_CALCULATE_SHADOWS_32(idx_light) { \
  RUN_CALCULATE_SHADOWS(idx_light, 0); \
  RUN_CALCULATE_SHADOWS(idx_light, 1); \
  RUN_CALCULATE_SHADOWS(idx_light, 2); \
  RUN_CALCULATE_SHADOWS(idx_light, 3); \
  RUN_CALCULATE_SHADOWS(idx_light, 4); \
  RUN_CALCULATE_SHADOWS(idx_light, 5); \
  RUN_CALCULATE_SHADOWS(idx_light, 6); \
  RUN_CALCULATE_SHADOWS(idx_light, 7); \
  RUN_CALCULATE_SHADOWS(idx_light, 8); \
  RUN_CALCULATE_SHADOWS(idx_light, 9); \
  RUN_CALCULATE_SHADOWS(idx_light, 10); \
  RUN_CALCULATE_SHADOWS(idx_light, 11); \
  RUN_CALCULATE_SHADOWS(idx_light, 12); \
  RUN_CALCULATE_SHADOWS(idx_light, 13); \
  RUN_CALCULATE_SHADOWS(idx_light, 14); \
  RUN_CALCULATE_SHADOWS(idx_light, 15); \
  RUN_CALCULATE_SHADOWS(idx_light, 16); \
  RUN_CALCULATE_SHADOWS(idx_light, 17); \
  RUN_CALCULATE_SHADOWS(idx_light, 18); \
  RUN_CALCULATE_SHADOWS(idx_light, 19); \
  RUN_CALCULATE_SHADOWS(idx_light, 20); \
  RUN_CALCULATE_SHADOWS(idx_light, 21); \
  RUN_CALCULATE_SHADOWS(idx_light, 22); \
  RUN_CALCULATE_SHADOWS(idx_light, 23); \
  RUN_CALCULATE_SHADOWS(idx_light, 24); \
  RUN_CALCULATE_SHADOWS(idx_light, 25); \
  RUN_CALCULATE_SHADOWS(idx_light, 26); \
  RUN_CALCULATE_SHADOWS(idx_light, 27); \
  RUN_CALCULATE_SHADOWS(idx_light, 28); \
  RUN_CALCULATE_SHADOWS(idx_light, 29); \
  RUN_CALCULATE_SHADOWS(idx_light, 30); \
  RUN_CALCULATE_SHADOWS(idx_light, 31); \
}

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

uniform vec3 entity_color;

in VS_OUT {
  vec3 normal;
  vec2 tex_coords;
  vec3 frag_position;
} fs_in;

layout (location = 0) out vec3 g_position;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec4 g_albedospec;

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
  vec3 unit_normal = normalize(fs_in.normal);

  vec3 entity_surface;
  if (n_diffuse_textures > 0) {
    entity_surface = texture(diffuse_textures[0], fs_in.tex_coords).rgb;
  } else {
    entity_surface = entity_color;
  }

  g_position = fs_in.frag_position;
  g_normal = unit_normal;
  g_albedospec.rgb = entity_surface;
  g_albedospec.a = texture(specular_textures[0], fs_in.tex_coords).r;
}
