#version 330 core

uniform sampler2D screen_texture;

in VS_OUT {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;

void main() {
  frag_color = vec4(
    1.0f - texture(screen_texture, fs_in.tex_coords).rgb,
    1.0f
  );
}
