#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 position;
in vec3 normal;
in vec2 tex_coords;

out vec3 vertex_normal;

void main() {
  gl_Position = projection * view * model * vec4(position, 1.0);
  vertex_normal = normal;
}
