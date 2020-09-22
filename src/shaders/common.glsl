#version 330 core

#define PI 3.14159265359

#define MAX_N_LIGHTS 8
#define MAX_N_SHADOW_FRAMEBUFFERS MAX_N_LIGHTS
#define MAX_N_TEXTURES 8

// NOTE: We need this hack because GLSL doesn't allow us to index samplerCubes
// by non-constant indices, so we can't do depth_textures[idx_light].
// Hopefully this will go away in the future (deferred lighting?).

#define RUN_CALCULATE_SHADOWS(frag_position, idx_light, idx_texture) { \
  if (idx_texture < n_depth_textures && idx_light == idx_texture) { \
    shadow += calculate_shadows(frag_position, idx_light, depth_textures[idx_texture]); \
  } \
}

#define RUN_CALCULATE_SHADOWS_ALL(frag_position, idx_light) { \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 0); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 1); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 2); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 3); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 4); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 5); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 6); \
  RUN_CALCULATE_SHADOWS(frag_position, idx_light, 7); \
}

struct Light {
  vec3 position;
  float pad_position;
  vec3 direction;
  float pad_direction;
  vec3 ambient;
  float pad_ambient;
  vec3 diffuse;
  float pad_diffuse;
  vec3 specular;
  float pad_specular;
  vec4 color;
  float attenuation_constant;
  float attenuation_linear;
  float attenuation_quadratic;
  float pad_attenuation;
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
