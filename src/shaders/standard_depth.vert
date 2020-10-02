uniform mat4 model;
uniform mat4 mesh_transform;

layout (location = 0) in vec3 position;

void main() {
  gl_Position = model * mesh_transform * vec4(position, 1.0);
}
