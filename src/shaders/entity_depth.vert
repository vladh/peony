#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 depth_view;
uniform mat4 depth_projection;

in vec3 position;
in vec3 normal;
in vec2 tex_coords;

void main() {
  gl_Position = depth_projection * depth_view * vec4(position, 1.0);
}
