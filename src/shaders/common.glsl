#version 330 core

#define PI 3.14159265359

#define MAX_N_LIGHTS 8
#define MAX_N_SHADOW_FRAMEBUFFERS MAX_N_LIGHTS

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

layout (std140) uniform shader_common {
  mat4 view;
  mat4 projection;
  mat4 shadow_transforms[6];

  vec3 camera_position;
  float pad_oops;

  float exposure;
  float t;
  float far_clip_dist;
  int n_lights;

  int shadow_light_idx;
  float pad_oops1;
  float pad_oops2;
  float pad_oops3;

  vec4 light_position[MAX_N_LIGHTS];
  vec4 light_color[MAX_N_LIGHTS];
  vec4 light_attenuation[MAX_N_LIGHTS];
};
