#version 330 core

in vec3 position;
in vec3 normal;
in vec2 tex_coords;

out VS_OUT {
  vec2 tex_coords;
} vs_out;

void main() {
  vs_out.tex_coords = tex_coords;
  gl_Position = vec4(position, 1.0);
}
