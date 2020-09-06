#version 330 core

in vec3 vertex_color;

out vec4 frag_color;

void main() {
  frag_color.rgb = vertex_color;
  frag_color.a = 1.0f;
}
