in BLOCK {
  vec2 tex_coords;
  vec4 color;
} fs_in;

out vec4 frag_color;

uniform sampler2D font_atlas_texture;

void main() {
  float opacity = texture(
    font_atlas_texture, fs_in.tex_coords
  ).r * fs_in.color.a;
  frag_color = vec4(fs_in.color.rgb, opacity);
}
