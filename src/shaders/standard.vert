uniform mat4 model;
uniform mat4 mesh_transform;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

out BLOCK {
  vec3 world_position;
  vec3 normal;
  vec2 tex_coords;
} vs_out;

void main() {
  vs_out.world_position = vec3(model * mesh_transform * vec4(position, 1.0));
  vs_out.normal = normalize(mat3(transpose(inverse(model * mesh_transform))) * normal);
  vs_out.tex_coords = tex_coords;
  gl_Position = projection * view * vec4(vs_out.world_position, 1.0);
}
