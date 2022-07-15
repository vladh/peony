// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

layout (location = 0) in vec3 position;
layout (location = 1) in vec4 color;

out BLOCK {
  vec4 color;
} vs_out;

void main() {
  gl_Position = projection * view * vec4(position, 1.0);
  vs_out.color = color;
}
