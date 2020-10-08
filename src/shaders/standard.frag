uniform sampler2DArray material_texture;

uniform vec4 albedo_static;
uniform float metallic_static;
uniform float roughness_static;
uniform float ao_static;

uniform bool should_use_normal_map;

in VS_OUT {
  vec3 frag_position;
  vec3 normal;
  vec2 tex_coords;
} fs_in;

layout (location = 0) out vec3 g_position;
layout (location = 1) out vec3 g_normal;
layout (location = 2) out vec4 g_albedo;
layout (location = 3) out vec4 g_pbr;

vec3 grid_sampling_offsets[20] = vec3[] (
  vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
  vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
  vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
  vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
  vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

// A simplified way to get our tangent-normals to world-space from LearnOpenGL.
// Don't really understand how this works!
// We probably want to convert this to the regular way of calculating them
// in the future, but since we're using both PBR and deferred lighting,
// it would be a bit troublesome to integrate into the code.
vec3 get_normal_from_map() {
  // TODO: Swizzle this differently.
  vec3 tangent_normal_rgb = texture(material_texture, vec3(fs_in.tex_coords, 4)).xyz * 2.0 - 1.0;
  vec3 tangent_normal = vec3(tangent_normal_rgb.b, tangent_normal_rgb.g, tangent_normal_rgb.r);

  vec3 Q1 = dFdx(fs_in.frag_position);
  vec3 Q2 = dFdy(fs_in.frag_position);
  vec2 st1 = dFdx(fs_in.tex_coords);
  vec2 st2 = dFdy(fs_in.tex_coords);

  vec3 N = normalize(fs_in.normal);
  vec3 T = normalize((Q1 * st2.t) - (Q2 * st1.t));
  vec3 B = -normalize(cross(N, T));
  mat3 TBN = mat3(T, B, N);

  return normalize(TBN * tangent_normal);
}

void main() {
  vec3 unit_normal = normalize(fs_in.normal);
  g_position = fs_in.frag_position;

  if (should_use_normal_map) {
    g_normal = get_normal_from_map();
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
