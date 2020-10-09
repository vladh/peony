in GS_OUT {
  vec3 world_position;
  vec3 normal;
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;

void main() {
  frag_color = vec4(0.0f, 0.0f, 1.0f, 1.0f);
}
