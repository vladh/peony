uniform sampler2D l_color_texture;
uniform sampler2D bloom_texture;
#if USE_FOG
  uniform sampler2D l_depth_texture;
#endif

in BLOCK {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;

void main() {
  vec3 color = texture(l_color_texture, fs_in.tex_coords).rgb;

  #if USE_BLOOM
    vec3 bloom = texture(bloom_texture, fs_in.tex_coords).rgb;
    color += bloom / 15.0;
  #endif

  #if USE_FOG
    float depth = texture(l_depth_texture, fs_in.tex_coords).r;
    float linear_depth = linearize_depth(
      depth, camera_near_clip_dist, camera_far_clip_dist
    ) / camera_far_clip_dist;
    float fog_term = pow(linear_depth, 3.0) * 3.0;
    color += FOG_ALBEDO * fog_term;
  #endif

  color = add_tone_mapping(color);
  color = correct_gamma(color);

  frag_color = vec4(color, 1.0);
}
