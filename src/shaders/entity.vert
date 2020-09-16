#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 position;
in vec3 normal;
in vec2 tex_coords;

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