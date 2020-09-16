#version 330 core

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
  float pad_11;
  vec3 depth_light_position;
  float pad_15;
  float t;
  float far_clip_dist;
  int n_lights;
  float pad_19;
  Light lights[8];
};

uniform mat4 model;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

void main() {
  gl_Position = projection * view * model * vec4(position, 1.0);
}
