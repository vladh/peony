#version 330 core

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

in vec3 position;
in vec3 normal;
in vec2 tex_coords;

out vec3 vertex_normal;
out vec3 frag_position;

void main() {
  frag_position = vec3(model * vec4(position, 1.0));
  vertex_normal = mat3(transpose(inverse(model))) * normal;
  gl_Position = projection * view * vec4(frag_position, 1.0);
}
