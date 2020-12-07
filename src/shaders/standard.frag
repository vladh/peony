uniform sampler2D albedo_texture;
uniform sampler2D metallic_texture;
uniform sampler2D roughness_texture;
uniform sampler2D ao_texture;
uniform sampler2D normal_texture;

uniform vec4 albedo_static;
uniform float metallic_static;
uniform float roughness_static;
uniform float ao_static;

uniform bool should_use_normal_map;

in BLOCK {
  vec3 world_position;
  vec3 normal;
  vec2 tex_coords;
} fs_in;

layout (location = 0) out vec3 g_position;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec4 g_albedo;
layout (location = 3) out vec4 g_pbr;

void main() {
  vec3 unit_normal = normalize(fs_in.normal);
  g_position = fs_in.world_position;

  if (should_use_normal_map) {
    g_normal = get_normal_from_map(
      texture(normal_texture, fs_in.tex_coords),
      fs_in.world_position,
      fs_in.normal,
      fs_in.tex_coords
    );
  } else {
    g_normal = unit_normal;
  }

  vec3 albedo;
  if (albedo_static.x < 0) {
    vec4 g_albedo_rgb = texture(albedo_texture, fs_in.tex_coords);
    g_albedo = vec4(g_albedo_rgb.z, g_albedo_rgb.y, g_albedo_rgb.x, g_albedo_rgb.a);
  } else {
    g_albedo = albedo_static;
  }

  float metallic;
  if (metallic_static < 0) {
    metallic = texture(metallic_texture, fs_in.tex_coords).r;
  } else {
    metallic = metallic_static;
  }

  float roughness;
  if (roughness_static < 0) {
    roughness = texture(roughness_texture, fs_in.tex_coords).r;
  } else {
    roughness = roughness_static;
  }

  float ao;
  if (ao_static < 0) {
    ao = texture(ao_texture, fs_in.tex_coords).r;
  } else {
    ao = ao_static;
  }

  g_pbr = vec4(metallic, roughness, ao, 1.0f);
}
