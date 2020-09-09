#version 330 core

uniform float t;

in vec3 vertex_normal;

out vec4 frag_color;

void main() {
  frag_color = vec4(vertex_normal, 1.0f);
}
