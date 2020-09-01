#version 330 core
out vec4 frag_color;
uniform float t;

void main() {
  frag_color = vec4(vec3(0.0f, 0.0f, 1.0f) * (sin(t * 4) + 1.5), 1.0f);
}
