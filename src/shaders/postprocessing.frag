uniform sampler2D l_color_texture;
uniform sampler2D bloom_texture;
// uniform sampler2D l_depth_texture;

in BLOCK {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;


void main() {
  vec3 color = texture(l_color_texture, fs_in.tex_coords).rgb;

  vec3 bloom = texture(bloom_texture, fs_in.tex_coords).rgb;
  color += bloom / 15.0;

  // float depth = texture(l_depth_texture, fs_in.tex_coords).r;
  // float linear_depth = linearize_depth(
  //   depth, camera_near_clip_dist, camera_far_clip_dist
  // ) / camera_far_clip_dist;
  // float fog_term = pow(linear_depth, 3.0) * 3.0;
  // color += FOG_ALBEDO * fog_term;

  color = add_tone_mapping(color);
  color = correct_gamma(color);

#if 0
  float aspect_ratio = window_width / window_height;
  vec2 preview_size = vec2(0.3, 0.3 / aspect_ratio);
  if (
    fs_in.tex_coords.x < preview_size.x &&
    fs_in.tex_coords.y < preview_size.y
  ) {
    vec2 preview_tex_coords = fs_in.tex_coords / preview_size;
    vec3 sample_p = vec3(vec2(1.0) - preview_tex_coords, 0.0);
    color = texture(texture_shadowmaps, sample_p).rrr;
  }
#endif

  frag_color = vec4(color, 1.0);
}
