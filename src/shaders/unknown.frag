out vec4 frag_color;

in BLOCK {
  vec3 normal;
} fs_in;

void main() {
  frag_color = vec4(1.0f, 0.0f, 1.0f, 1.0f);
}
