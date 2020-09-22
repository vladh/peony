uniform int n_diffuse_textures;
uniform sampler2D diffuse_textures[MAX_N_TEXTURES];
uniform int n_specular_textures;
uniform sampler2D specular_textures[MAX_N_TEXTURES];
uniform int n_depth_textures;
uniform samplerCube depth_textures[MAX_N_SHADOW_FRAMEBUFFERS];

uniform vec4 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

in VS_OUT {
  vec3 normal;
  vec2 tex_coords;
  vec3 frag_position;
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

float calculate_shadows(vec3 frag_position, int idx_light, samplerCube depth_texture) {
  vec3 frag_to_light = frag_position - lights[idx_light].position;
  float current_depth = length(frag_to_light);

  float shadow = 0.0f;
  float bias = 0.30f;
  int n_samples = 20;

  float view_distance = length(camera_position - frag_position);
  float sample_radius = (1.0f + (view_distance / far_clip_dist)) / 25.0f;

  for (int i = 0; i < n_samples; i++) {
    float closest_depth = texture(
      depth_texture,
      frag_to_light + grid_sampling_offsets[i] * sample_radius
    ).r * far_clip_dist;

    if (current_depth - bias > closest_depth) {
      shadow += 1.0;
    }
  }
  shadow /= float(n_samples);

  return shadow;
}

void main() {
  vec3 unit_normal = normalize(fs_in.normal);

#if 0
  vec3 entity_color = vec3(1.0f, 0.0f, 0.0f);

  vec3 entity_surface;
  if (n_diffuse_textures > 0) {
    entity_surface = texture(diffuse_textures[0], fs_in.tex_coords).rgb;
  } else {
    entity_surface = entity_color;
  }
#endif

  g_position = fs_in.frag_position;
  g_normal = unit_normal;
  g_albedo = albedo;
  g_pbr = vec4(metallic, roughness, ao, 1.0f);
}
