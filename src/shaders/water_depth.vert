uniform mat4 model_matrix;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

out BLOCK {
  vec3 world_position;
  vec3 normal;
  vec2 tex_coords;
} vs_out;

void main() {
  vec3 prelim_world_position = vec3(model_matrix * vec4(position, 1.0));
  vec3 water_position = water_make_position(prelim_world_position.xz);
  gl_Position = vec4(water_position, 1.0);
}
