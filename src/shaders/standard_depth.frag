in BLOCK {
  vec4 world_position;
} fs_in;

void main() {
  vec4 current_light_position;

  if (current_shadow_light_type == LIGHT_POINT) {
    current_light_position = point_light_position[current_shadow_light_idx];
  } else if (current_shadow_light_type == LIGHT_DIRECTIONAL) {
    current_light_position = directional_light_position[current_shadow_light_idx];
  }

  float light_distance = length(
    fs_in.world_position.xyz - vec3(current_light_position)
  ) / shadow_far_clip_dist;

  gl_FragDepth = light_distance;
}
