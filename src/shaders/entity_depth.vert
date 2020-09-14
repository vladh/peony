#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 position;
in vec3 normal;
in vec2 tex_coords;

void main() {
  gl_Position = projection * view * vec4(position, 1.0);
}
