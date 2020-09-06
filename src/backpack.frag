#version 330 core

uniform sampler2D texture_diffuse1;

in vec2 vertex_tex_coords;

out vec4 frag_color;

void main() {
  frag_color = texture(texture_diffuse1, vertex_tex_coords);
}
