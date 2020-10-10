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
  vec3 prelim_world_position = vec3(model * mesh_transform * vec4(position, 1.0));

  vec3 water_position = water_make_position(prelim_world_position.xz);
  vec3 water_normal = water_make_normal(water_position);

  gl_Position = projection * view * vec4(water_position, 1.0);
  vs_out.world_position = water_position;
  vs_out.normal = normalize(water_normal);
  vs_out.tex_coords = tex_coords;
}
