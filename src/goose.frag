#version 330 core

uniform float t;

in vec3 vertex_normal;

out vec4 frag_color;

void main() {
  /* frag_color = vec4(vec3(0.0f, 0.0f, 1.0f) * (sin(t * 4) + 1.5), 1.0f); */
  frag_color.rgb = vertex_normal;
  frag_color.a = 1.0f;
}
