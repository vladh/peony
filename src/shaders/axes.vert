uniform mat4 model;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

out vec3 vertex_normal;

void main() {
  gl_Position = projection * view * model * vec4(position, 1.0);
  vertex_normal = normal;
}
