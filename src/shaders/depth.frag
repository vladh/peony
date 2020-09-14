#version 330 core

uniform sampler2D texture_diffuse1;
uniform float near_clip_dist;
uniform float far_clip_dist;

in VS_OUT {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;

// Required when using a perspective projection matrix
float linearize_depth(float depth) {
  float z = depth * 2.0 - 1.0; // Back to NDC
  return (2.0 * near_clip_dist * far_clip_dist) /
    (far_clip_dist + near_clip_dist - z *
    (far_clip_dist - near_clip_dist));
}

void main() {
  float depth_value = texture(texture_diffuse1, fs_in.tex_coords).r;
  frag_color = vec4(vec3(depth_value), 1.0f);
}
