in BLOCK {
  vec4 color;
} fs_in;

out vec4 frag_color;

void main() {
  frag_color = fs_in.color;
}
