uniform int shadow_light_idx;

in GS_OUT {
  vec4 frag_position;
} fs_in;

void main() {
  float light_distance = length(
    fs_in.frag_position.xyz - vec3(light_position[shadow_light_idx])
  ) / far_clip_dist;
  gl_FragDepth = light_distance;
}
