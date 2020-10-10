#version 330 core

#define PI 3.14159265359
#define MAX_N_LIGHTS 8
#define MAX_N_SHADOW_FRAMEBUFFERS MAX_N_LIGHTS

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
