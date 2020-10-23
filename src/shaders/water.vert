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

  vec3 water_position = water_make_position(prelim_world_position.xz);
  vec3 water_normal;
  vec3 water_bitangent;
  vec3 water_tangent;
  water_make_normal(water_position, water_normal, water_bitangent, water_tangent);

  gl_Position = projection * view * vec4(water_position, 1.0);

  vec3 ndc = gl_Position.xyz / gl_Position.w;
  vs_out.screen_position = ndc.xy * 0.5 + 0.5;

  vs_out.world_position = water_position;
  vs_out.normal = normalize(water_normal);
  vs_out.bitangent = water_bitangent;
  vs_out.tangent = water_tangent;
  vs_out.tex_coords = tex_coords;
}
