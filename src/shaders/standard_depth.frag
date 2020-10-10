in BLOCK {
  vec4 world_position;
} fs_in;

void main() {
  float light_distance = length(
    fs_in.world_position.xyz - vec3(light_position[shadow_light_idx])
  ) / far_clip_dist;
  gl_FragDepth = light_distance;
}
