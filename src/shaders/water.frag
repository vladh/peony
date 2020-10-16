uniform sampler2D g_albedo_texture;

in BLOCK {
  vec3 world_position;
  vec2 screen_position;
  vec3 normal;
  vec2 tex_coords;
} fs_in;

layout (location = 0) out vec4 frag_color;

void main() {
  vec3 unit_normal = normalize(fs_in.normal);
  vec3 water_color = vec3(0.0, 0.0, 0.5);
  vec3 water_albedo = vec3(0.1, 0.1, 1.0);
  vec3 underwater_albedo = linearize_albedo(
    texture(g_albedo_texture, fs_in.screen_position).rgb
  );
  vec3 refraction_color = desaturate(underwater_albedo, 0.5) * water_albedo;

  vec3 pbr_light = compute_pbr_light(
    water_albedo, 0.0, 1.0, 1.0,
    fs_in.world_position, fs_in.normal
  );
  frag_color = vec4(pbr_light + water_color + refraction_color, 1.0);
}
