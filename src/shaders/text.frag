in vec2 tex_coords;

out vec4 color;

uniform sampler2D atlas_texture;
uniform vec4 text_color;

void main() {
  vec4 sampled = vec4(1.0, 1.0, 1.0, texture(atlas_texture, tex_coords).r);
  color = text_color * sampled;
}
