#version 330 core

uniform float t;

uniform sampler2D texture_diffuse1;

in vec3 vertex_color;
in vec2 vertex_tex_coords;

out vec4 frag_color;

void main() {
  frag_color = texture(texture_diffuse1, vertex_tex_coords) *
    vec4(vertex_color * (sin(t * 2) + 1), 1.0f);
}
