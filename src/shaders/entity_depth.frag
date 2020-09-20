#version 330 core

#define MAX_N_LIGHTS 8

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
  float pad_17;
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

uniform int shadow_light_idx;

in GS_OUT {
  vec4 frag_position;
} fs_in;

void main() {
  float light_distance = length(
    fs_in.frag_position.xyz - lights[shadow_light_idx].position
  ) / far_clip_dist;
  gl_FragDepth = light_distance;
}
