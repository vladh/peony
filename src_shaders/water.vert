// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

uniform mat4 model_matrix;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

out BLOCK {
  vec3 world_position;
  vec3 normal;
#if SHOULD_CALCULATE_TANGENT_IN_VERTEX_SHADER
  vec3 bitangent;
  vec3 tangent;
#endif
  vec2 tex_coords;
} vs_out;


void main() {
  vec3 prelim_world_position = vec3(model_matrix * vec4(position, 1.0));

  vec3 water_lf_position = water_make_position(prelim_world_position.xz);
  vec3 water_lf_normal;
  vec3 water_lf_bitangent;
  vec3 water_lf_tangent;
  water_make_normal(water_lf_position, water_lf_normal, water_lf_bitangent, water_lf_tangent);

  vec2 noise_freq = vec2(6.5, 6.5);
  vec2 noise_time_term = vec2(t, 0.0);
  float noise_amplitude = 0.03;
  vec3 noise_terms = noised(prelim_world_position.xz * noise_freq + noise_time_term);
  float noise_height = noise_terms.x * noise_amplitude;
  vec2 noise_dy = noise_terms.yz * noise_amplitude * noise_freq;
  vec3 water_hf_position = prelim_world_position + vec3(0.0, noise_height, 0.0);
#if SHOULD_CALCULATE_TANGENT_IN_VERTEX_SHADER
  vec3 water_hf_bitangent = normalize(vec3(1.0, noise_dy[0], 0.0));
  vec3 water_hf_tangent = normalize(vec3(0.0, noise_dy[1], 1.0));
#endif
  vec3 water_hf_normal = normalize(vec3(-noise_dy[0], 1.0, -noise_dy[1]));

  vec3 water_position = water_lf_position + vec3(0.0, water_hf_position.y, 0.0);
  vec3 water_normal = normalize(water_hf_normal + water_lf_normal);
#if SHOULD_CALCULATE_TANGENT_IN_VERTEX_SHADER
  vec3 water_tangent = normalize(water_hf_tangent + water_lf_tangent);
  vec3 water_bitangent = normalize(water_hf_bitangent + water_lf_bitangent);
#endif

  gl_Position = projection * view * vec4(water_position, 1.0);

  vs_out.world_position = water_position;
  vs_out.normal = water_normal;
#if SHOULD_CALCULATE_TANGENT_IN_VERTEX_SHADER
  vs_out.bitangent = water_bitangent;
  vs_out.tangent = water_tangent;
#endif
  vs_out.tex_coords = tex_coords;
}
