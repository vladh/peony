#version 330 core
out vec4 frag_color;
in vec3 vertex_color;
in vec2 vertex_tex_coord;
uniform sampler2D frag_texture;

void main() {
  /* frag_color = texture(frag_texture, vertex_tex_coord) * vec4(vertex_color, 1.0f); */
  frag_color = vec4(vertex_color, 1.0f);
}
