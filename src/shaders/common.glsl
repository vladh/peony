#version 410 core

#define M_PI 3.14159265358979323846
#define M_EPSILON 1e-5
#define MAX_N_LIGHTS 8

#define LIGHT_POINT 0
#define LIGHT_DIRECTIONAL 1

layout (std140) uniform shader_common {
  mat4 view;
  mat4 projection;
  mat4 shadow_transforms[6 * MAX_N_LIGHTS];

  vec3 camera_position;
  float camera_pitch;

  float camera_horizontal_fov;
  float camera_vertical_fov;
  int shadow_light_idx;
  float pad_oops1;

  float exposure;
  float t;
  float far_clip_dist;
  int n_lights;

  vec4 light_position[MAX_N_LIGHTS];
  vec4 light_type[MAX_N_LIGHTS];
  vec4 light_direction[MAX_N_LIGHTS];
  vec4 light_color[MAX_N_LIGHTS];
  vec4 light_attenuation[MAX_N_LIGHTS];
};

struct WaveParameterSet {
  vec2 direction;
  float amplitude;
  float steepness;
  float frequency;
  float speed;
};

int n_waves = 8;
WaveParameterSet wave_parameter_sets[8] = WaveParameterSet[8](
  // Choppy small waves
  WaveParameterSet(vec2(0.8, 0.1), 0.03, 0.001, 14.8, 0.2),
  WaveParameterSet(vec2(0.4, 0.5), 0.02, 0.003, 15.2, 0.3),
  WaveParameterSet(vec2(0.0, 1.0), 0.05, 0.002, 3.3, 0.4),
  // Big waves
  WaveParameterSet(vec2(0.0, 0.8), 0.5, 0.41, 0.8, 0.8),
  WaveParameterSet(vec2(0.1, 0.7), 0.3, 0.25, 0.9, 1.5),
  WaveParameterSet(vec2(0.75, 0.1), 0.2, 0.12, 0.8, 0.5),
  WaveParameterSet(vec2(0.5, 0.5), 0.2, 0.20, 0.3, 0.1),
  // Slow large waves
  WaveParameterSet(vec2(0.5, 0.5), 0.1, 0.12, 0.12, 0.25)
);

vec3 water_make_normal_gerstner_osgw(vec3 water_position) {
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

vec3 water_make_position_gerstner_osgw(vec2 vertex_position) {
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

// (x, y) = vertex_position
// Q = steepness
// A = amplitude
// D = direction
// L = wavelength
// S = speed
// w = frequency = 2 / L
// phi = phase-constant = S * w
vec3 water_make_position_gerstner_vlad(vec2 vertex_position) {
  vec3 wave_position = vec3(vertex_position.x, 0, vertex_position.y);

  for (int idx = 0; idx < n_waves; idx++) {
    float amplitude = wave_parameter_sets[idx].amplitude;
    float frequency = wave_parameter_sets[idx].frequency;
    vec2 direction = wave_parameter_sets[idx].direction;
    float speed = wave_parameter_sets[idx].speed;
    float phi = speed * frequency;
    float steepness = wave_parameter_sets[idx].steepness;

    float sin_cos_argument = dot(frequency * direction, vertex_position) + phi * t;

    wave_position.x += steepness * amplitude * direction.x *
      cos(sin_cos_argument);
    wave_position.z += steepness * amplitude * direction.y *
      cos(sin_cos_argument);
    wave_position.y += amplitude *
      sin(sin_cos_argument);
  }

  return wave_position;
}

// P = wave position
// WA = w * A
// S() = sin term
// C() = cos term
vec3 water_make_normal_gerstner_vlad(
  vec3 wave_position, out vec3 normal, out vec3 bitangent, out vec3 tangent
) {
  normal = vec3(0.0, 1.0, 0.0);
  bitangent = vec3(1.0, 0.0, 0.0);
  tangent = vec3(0.0, 0.0, 1.0);

  for (int idx = 0; idx < n_waves; idx++) {
    float amplitude = wave_parameter_sets[idx].amplitude;
    float frequency = wave_parameter_sets[idx].frequency;
    vec2 direction = wave_parameter_sets[idx].direction;
    float speed = wave_parameter_sets[idx].speed;
    float phi = speed * frequency;
    float steepness = wave_parameter_sets[idx].steepness;

    float wa = frequency * amplitude;
    float sin_cos_argument = frequency * dot(direction, wave_position.xz) + phi * t;
    float sin_term = sin(sin_cos_argument);
    float cos_term = cos(sin_cos_argument);

    normal.x -= direction.x * wa * cos_term;
    normal.z -= direction.y * wa * cos_term;
    normal.y -= steepness * wa * sin_term;

    bitangent.x -= steepness * pow(direction.x, 2) * wa * sin_term;
    bitangent.y += direction.x * wa * cos_term;
    bitangent.z -= steepness * direction.x * direction.y * wa * sin_term;

    tangent.x -= steepness * direction.x * direction.y * wa * sin_term;
    tangent.y += direction.y * wa * cos_term;
    tangent.z -= steepness * pow(direction.y, 2) * wa * sin_term;
  }

  return normal;
}

vec3 water_make_position(vec2 vertex_position) {
  return water_make_position_gerstner_vlad(vertex_position);
}

vec3 water_make_normal(
  vec3 water_position, inout vec3 normal, inout vec3 bitangent, inout vec3 tangent
) {
  return water_make_normal_gerstner_vlad(water_position, normal, bitangent, tangent);
}

float to_unit_interval(float x, float min_val, float max_val) {
  return clamp(
    (x - min_val) / (max_val - min_val),
    0, 1
  );
}

float hash(vec2 p) {
  // https://stackoverflow.com/q/12964279/3803222
  float h = dot(p, vec2(127.1,311.7));
  return fract(sin(h) * 43758.5453123);
}
