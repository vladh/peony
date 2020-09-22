uniform int n_depth_textures;
uniform samplerCube depth_textures[MAX_N_SHADOW_FRAMEBUFFERS];

uniform sampler2D albedo_texture;
uniform sampler2D metallic_texture;
uniform sampler2D roughness_texture;
uniform sampler2D ao_texture;

uniform vec4 albedo_static;
uniform float metallic_static;
uniform float roughness_static;
uniform float ao_static;

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

float calculate_shadows(vec3 frag_position, int idx_light, samplerCube depth_texture) {
  vec3 frag_to_light = frag_position - vec3(lights[idx_light].position);
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
  g_position = fs_in.frag_position;
  g_normal = unit_normal;

  vec3 albedo;
  if (albedo_static.x < 0) {
    g_albedo = texture(albedo_texture, fs_in.tex_coords);
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
