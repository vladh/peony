uniform mat4 model;
uniform mat4 mesh_transform;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

void main() {
  gl_Position = projection * view * model * mesh_transform * vec4(position, 1.0);
}
