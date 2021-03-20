in BLOCK {
  vec3 normal;
} fs_in;

out vec4 frag_color;

void main() {
  frag_color = vec4(normal, 1.0f);
}
