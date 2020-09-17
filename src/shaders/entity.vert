#version 330 core

#define MAX_N_LIGHTS 32

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

uniform mat4 model;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

out VS_OUT {
  vec3 normal;
  vec2 tex_coords;
  vec3 frag_position;
} vs_out;

void main() {
  vs_out.frag_position = vec3(model * vec4(position, 1.0));
  vs_out.normal = mat3(transpose(inverse(model))) * normal;
  vs_out.tex_coords = tex_coords;
  gl_Position = projection * view * vec4(vs_out.frag_position, 1.0);
}
