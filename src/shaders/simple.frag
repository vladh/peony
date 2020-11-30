out vec4 frag_color;

in BLOCK {
  vec3 normal;
} fs_in;

void main() {
  vec3 N = normalize(fs_in.normal);
  frag_color = vec4(abs(N), 1.0f);
}
