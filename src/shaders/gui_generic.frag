in BLOCK {
  vec2 tex_coords;
  vec4 color;
} fs_in;

out vec4 frag_color;

uniform sampler2D atlas_texture;

void main() {
  float opacity = 1.0f;

  if (fs_in.tex_coords.x > 0.0 && fs_in.tex_coords.y > 0.0f) {
    opacity = texture(
      atlas_texture, fs_in.tex_coords
    ).r;
  }

  frag_color = vec4(fs_in.color.rgb, opacity * fs_in.color.a);
}
