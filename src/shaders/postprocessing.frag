#version 330 core

uniform sampler2D texture_diffuse1;

in VS_OUT {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;

void main() {
  frag_color = vec4(
    1.0f - texture(texture_diffuse1, fs_in.tex_coords).rgb,
    1.0f
  );
}
