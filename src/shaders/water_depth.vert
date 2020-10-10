uniform mat4 model;
uniform mat4 mesh_transform;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

out VS_OUT {
  vec3 world_position;
  vec3 normal;
  vec2 tex_coords;
} vs_out;

void main() {
  n_waves = 4;

  wave_parameter_sets[0].direction = vec2(1.0, 0.0);
  wave_parameter_sets[0].amplitude = 0.8;
  wave_parameter_sets[0].steepness = 0.2;
  wave_parameter_sets[0].frequency = 1.0;
  wave_parameter_sets[0].speed = 1.0;

  wave_parameter_sets[1].direction = vec2(0.8, 0.0);
  wave_parameter_sets[1].amplitude = 0.4;
  wave_parameter_sets[1].steepness = 0.3;
  wave_parameter_sets[1].frequency = 4.0;
  wave_parameter_sets[1].speed = 1.5;

  wave_parameter_sets[2].direction = vec2(0.0, 1.0);
  wave_parameter_sets[2].amplitude = 0.2;
  wave_parameter_sets[2].steepness = 0.5;
  wave_parameter_sets[2].frequency = 2.0;
  wave_parameter_sets[2].speed = 0.8;

  wave_parameter_sets[3].direction = vec2(0.0, 0.7);
  wave_parameter_sets[3].amplitude = 0.2;
  wave_parameter_sets[3].steepness = 0.2;
  wave_parameter_sets[3].frequency = 1.5;
  wave_parameter_sets[3].speed = 0.6;

  vec3 prelim_world_position = vec3(model * mesh_transform * vec4(position, 1.0));

  vec3 water_position = make_water_position(prelim_world_position.xz);
  vec3 water_normal = make_water_normal(water_position);

  gl_Position = vec4(water_position, 1.0);
}
