#version 330 core

uniform vec3 depth_light_position;
uniform float far_clip_dist;

in GS_OUT {
  vec4 frag_position;
} fs_in;

void main() {
  float light_distance = length(
    fs_in.frag_position.xyz - depth_light_position
  ) / far_clip_dist;
  gl_FragDepth = light_distance;
}
