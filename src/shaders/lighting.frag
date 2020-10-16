uniform sampler2D g_position_texture;
uniform sampler2D g_normal_texture;
uniform sampler2D g_albedo_texture;
uniform sampler2D g_pbr_texture;

in BLOCK {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;

void main() {
  vec3 world_position = texture(g_position_texture, fs_in.tex_coords).rgb;
  vec3 normal = texture(g_normal_texture, fs_in.tex_coords).rgb;

  // Skip pixels with no normal. These are the background pixels, so we
  // let the background color shine through.
  if (normal == vec3(0.0, 0.0, 0.0)) {
    discard;
  }

  vec3 albedo = linearize_albedo(texture(g_albedo_texture, fs_in.tex_coords).rgb);
  vec4 pbr_texture = texture(g_pbr_texture, fs_in.tex_coords);
  float metallic = pbr_texture.r;
  float roughness = pbr_texture.g;
  float ao = pbr_texture.b;

  vec3 color = compute_pbr_light(
    albedo, metallic, roughness, ao,
    world_position, normal
  );

  color = add_tone_mapping(color);
  color = correct_gamma(color);

  frag_color = vec4(color, 1.0);
}
