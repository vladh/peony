uniform sampler2D g_albedo_texture;

in BLOCK {
  vec3 world_position;
  vec4 screen_position;
  vec3 normal;
  vec2 tex_coords;
} fs_in;

layout (location = 0) out vec4 frag_color;

void main() {
  vec3 unit_normal = normalize(fs_in.normal);
  frag_color = vec4(0.1, 0.1, 0.9, 0.8);
  frag_color = texture(g_albedo_texture, fs_in.tex_coords * fs_in.screen_position.xy);
}
