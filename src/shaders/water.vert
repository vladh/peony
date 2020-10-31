uniform mat4 model;
uniform mat4 mesh_transform;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

out BLOCK {
  vec3 world_position;
  vec2 screen_position;
  vec3 normal;
  vec3 bitangent;
  vec3 tangent;
  vec2 tex_coords;
} vs_out;


void main() {
  vec3 prelim_world_position = vec3(model * mesh_transform * vec4(position, 1.0));

  vec3 water_lf_position = water_make_position(prelim_world_position.xz);
  vec3 water_lf_normal;
  vec3 water_lf_bitangent;
  vec3 water_lf_tangent;
  water_make_normal(water_lf_position, water_lf_normal, water_lf_bitangent, water_lf_tangent);

  vec2 noise_freq = vec2(1.5, 0.5);
  float noise_amplitude = 0.3;
  vec3 noise_terms = noised(prelim_world_position.xz * noise_freq);
  float noise_height = noise_terms.x * noise_amplitude;
  vec2 noise_dy = noise_terms.yz * noise_amplitude * noise_freq;
  vec3 water_hf_position = prelim_world_position + vec3(0.0, noise_height, 0.0);
  vec3 water_hf_bitangent = normalize(vec3(1.0, noise_dy[0], 0.0));
  vec3 water_hf_tangent = normalize(vec3(0.0, noise_dy[1], 1.0));
  vec3 water_hf_normal = normalize(vec3(-noise_dy[0], 1.0, -noise_dy[1]));

  vec3 water_position = water_lf_position + vec3(0.0, water_hf_position.y, 0.0);
  vec3 water_normal = normalize(water_hf_normal + water_lf_normal);
  vec3 water_tangent = normalize(water_hf_tangent + water_lf_tangent);
  vec3 water_bitangent = normalize(water_hf_bitangent + water_lf_bitangent);

  /* vec3 water_position = water_hf_position; */
  /* vec3 water_normal = water_hf_normal; */
  /* vec3 water_tangent = water_hf_tangent; */
  /* vec3 water_bitangent = water_hf_bitangent; */

  /* vec3 water_position = water_lf_position; */
  /* vec3 water_normal = water_lf_normal; */
  /* vec3 water_tangent = water_lf_tangent; */
  /* vec3 water_bitangent = water_lf_bitangent; */

  gl_Position = projection * view * vec4(water_position, 1.0);

  vec3 ndc = gl_Position.xyz / gl_Position.w;
  vs_out.screen_position = ndc.xy * 0.5 + 0.5;

  vs_out.world_position = water_position;
  vs_out.normal = water_normal;
  vs_out.bitangent = water_bitangent;
  vs_out.tangent = water_tangent;
  vs_out.tex_coords = tex_coords;
}
