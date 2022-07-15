// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

in BLOCK {
  vec3 normal;
} fs_in;

out vec4 frag_color;

void main() {
  frag_color = vec4(fs_in.normal, 1.0);
}
