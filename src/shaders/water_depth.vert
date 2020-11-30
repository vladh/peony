uniform mat4 model_matrix;

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 tex_coords;

out BLOCK {
  vec4 world_position;
} vs_out;

void main() {
  vec3 prelim_world_position = vec3(model_matrix * vec4(position, 1.0));
  vec3 water_lf_position = water_make_position(prelim_world_position.xz);
  vs_out.world_position = vec4(water_lf_position, 1.0);

  // NOTE: The water is only affected by sunlight.
  mat4 shadow_transform = texture_shadowmap_transforms[0];

  gl_Position = shadow_transform * vs_out.world_position;
}
