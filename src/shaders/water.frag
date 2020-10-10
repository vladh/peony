uniform sampler2DArray material_texture;

uniform vec4 albedo_static;
uniform float metallic_static;
uniform float roughness_static;
uniform float ao_static;

uniform bool should_use_normal_map;

in VS_OUT {
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
      texture(material_texture, vec3(fs_in.tex_coords, 4)),
      fs_in.world_position,
      fs_in.normal,
      fs_in.tex_coords
    );
  } else {
    g_normal = unit_normal;
  }

  vec3 albedo;
  if (albedo_static.x < 0) {
    // TODO: Swizzle this differently.
    vec4 g_albedo_rgb = texture(material_texture, vec3(fs_in.tex_coords, 0));
    g_albedo = vec4(g_albedo_rgb.z, g_albedo_rgb.y, g_albedo_rgb.x, g_albedo_rgb.a);
  } else {
    g_albedo = albedo_static;
  }

  float metallic;
  if (metallic_static < 0) {
    metallic = texture(material_texture, vec3(fs_in.tex_coords, 1)).r;
  } else {
    metallic = metallic_static;
  }

  float roughness;
  if (roughness_static < 0) {
    roughness = texture(material_texture, vec3(fs_in.tex_coords, 2)).r;
  } else {
    roughness = roughness_static;
  }

  float ao;
  if (ao_static < 0) {
    ao = texture(material_texture, vec3(fs_in.tex_coords, 3)).r;
  } else {
    ao = ao_static;
  }

  g_pbr = vec4(metallic, roughness, ao, 1.0f);
}
