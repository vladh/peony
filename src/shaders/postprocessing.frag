uniform sampler2D l_color_texture;
uniform sampler2D bloom_texture;

in BLOCK {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;


void main() {
  vec3 color = texture(l_color_texture, fs_in.tex_coords).rgb;
  vec3 bloom = texture(bloom_texture, fs_in.tex_coords).rgb;
  color += bloom / 15.0;
  color = add_tone_mapping(color);
  color = correct_gamma(color);
  frag_color = vec4(color, 1.0);
}
