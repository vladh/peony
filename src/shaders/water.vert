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


struct WaveParameterSet {
  vec2 direction;
  float amplitude;
  float steepness;
  float frequency;
  float speed;
};

/* uniform int n_waves = 0; */
/* uniform WaveParameterSet wave_parameter_sets[10]; */

int n_waves;
WaveParameterSet wave_parameter_sets[10];

vec3 make_water_normal(vec3 water_position) {
  vec3 wave_normal = vec3(0.0, 1.0, 0.0);

  for (int idx = 0; idx < n_waves; idx++) {
    float proj = dot(water_position.xz, wave_parameter_sets[idx].direction);
    float phase = t * wave_parameter_sets[idx].speed;
    float psi = proj * wave_parameter_sets[idx].frequency + phase;
    float amp_freq = wave_parameter_sets[idx].amplitude *
      wave_parameter_sets[idx].frequency;
    float alpha = amp_freq * sin(psi);

    wave_normal.y -= wave_parameter_sets[idx].steepness * alpha;

    float x = wave_parameter_sets[idx].direction.x;
    float y = wave_parameter_sets[idx].direction.y;
    float omega = amp_freq * cos(psi);

    wave_normal.x -= x * omega;
    wave_normal.z -= y * omega;
  }

  return wave_normal;
}

vec3 make_water_position(vec2 vertex_position) {
  vec3 wave_position = vec3(vertex_position.x, 0, vertex_position.y);

  for (int idx = 0; idx < n_waves; idx++) {
    float proj = dot(vertex_position, wave_parameter_sets[idx].direction);
    float phase = t * wave_parameter_sets[idx].speed;
    float theta = proj * wave_parameter_sets[idx].frequency + phase;
    float height = wave_parameter_sets[idx].amplitude * sin(theta);

    wave_position.y += height;

    float maximum_width = wave_parameter_sets[idx].steepness *
      wave_parameter_sets[idx].amplitude;
    float width = maximum_width * cos(theta);
    float x = wave_parameter_sets[idx].direction.x;
    float y = wave_parameter_sets[idx].direction.y;

    wave_position.x += x * width;
    wave_position.z += y * width;
  }

  return wave_position;
}

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

  gl_Position = projection * view * vec4(water_position, 1.0);
  vs_out.world_position = water_position;
  /* vs_out.normal = water_normal; */
  vs_out.normal = vec3(0.0, 1.0, 0.0);
  vs_out.tex_coords = tex_coords;
}
