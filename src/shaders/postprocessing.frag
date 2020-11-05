uniform sampler2D l_color_texture;

in BLOCK {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;


void main() {
  vec3 color = vec3(0.0);

  color = texture(l_color_texture, fs_in.tex_coords).rgb;
  color = add_tone_mapping(color);
  color = correct_gamma(color);

  frag_color = vec4(color, 1.0);
}
