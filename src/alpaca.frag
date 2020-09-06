#version 330 core

uniform float t;

uniform sampler2D texture_diffuse1;

in vec2 vertex_tex_coords;

out vec4 frag_color;

void main() {
  frag_color = texture(texture_diffuse1, vertex_tex_coords) *
    vec4(vec3(1.0f, 1.0f, 1.0f) * (sin(t * 2) + 1), 1.0f);
}
