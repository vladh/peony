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

layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;

out GS_OUT {
  vec4 frag_position;
} gs_out;

void main() {
  for(int face = 0; face < 6; face++) {
    // built-in variable that specifies to which face we render.
    gl_Layer = face;

    // for each triangle's vertices
    for(int i = 0; i < 3; ++i) {
      gs_out.frag_position = gl_in[i].gl_Position;
      gl_Position = shadow_transforms[face] * gs_out.frag_position;
      EmitVertex();
    }

    EndPrimitive();
  }
}
