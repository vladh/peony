#define GAMMA 2.2

uniform int n_diffuse_textures;
uniform sampler2D diffuse_textures[MAX_N_TEXTURES];
uniform int n_specular_textures;
uniform sampler2D specular_textures[MAX_N_TEXTURES];
uniform int n_depth_textures;
uniform samplerCube depth_textures[MAX_N_SHADOW_FRAMEBUFFERS];

uniform float exposure;

in VS_OUT {
  vec2 tex_coords;
} fs_in;

out vec4 frag_color;

vec3 invert(vec3 rgb) {
  return 1 - rgb;
}

vec3 correct_gamma(vec3 rgb) {
  return pow(rgb, vec3(1.0 / GAMMA));
}

vec3 add_tone_mapping(vec3 rgb) {
  // Reinhard tone mapping
  /* return rgb / (rgb + vec3(1.0)); */
  // Exposure tone mapping
  return vec3(1.0) - exp(-rgb * exposure);
}

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
  // retrieve data from G-buffer
  vec3 frag_position = texture(diffuse_textures[0], fs_in.tex_coords).rgb;
  vec3 normal = texture(diffuse_textures[1], fs_in.tex_coords).rgb;
  vec3 diffuse_texture_0 = texture(diffuse_textures[2], fs_in.tex_coords).rgb;
  float specular_texture_0 = texture(diffuse_textures[2], fs_in.tex_coords).a;

  if (normal == vec3(0.0f, 0.0f, 0.0f)) {
    discard;
  }

  vec3 unit_normal = normalize(normal);
  vec3 lighting = vec3(0.0f, 0.0f, 0.0f);

  for (int idx_light = 0; idx_light < n_lights; idx_light++) {
    Light light = lights[idx_light];

    // Ambient
    vec3 ambient = light.ambient;

    // Diffuse
    vec3 light_dir = normalize(light.position - frag_position);
    float diffuse_intensity = max(dot(unit_normal, light_dir), 0.0f);
    vec3 diffuse = light.diffuse * diffuse_intensity;

    // Specular
    vec3 view_dir = normalize(camera_position - frag_position);
    vec3 reflect_dir = reflect(-light_dir, unit_normal);
    float specular_intensity = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = light.specular * specular_intensity;

    // Attenuation
    float distance = length(light.position - frag_position);
    float attenuation = 1.0 / (
      light.attenuation_constant +
      light.attenuation_linear * distance +
      light.attenuation_quadratic * (distance * distance)
    );
    diffuse *= attenuation;
    specular *= attenuation;

    if (n_depth_textures >= n_lights && USE_SHADOWS) {
      float shadow = 0;
      RUN_CALCULATE_SHADOWS_ALL(frag_position, idx_light);
      lighting += (ambient + ((1.0f - shadow) * (diffuse + specular))) * diffuse_texture_0;
    } else {
      lighting += (ambient + diffuse + specular) * diffuse_texture_0;
    }
  }

  lighting = add_tone_mapping(lighting);
  lighting = correct_gamma(lighting);

  frag_color = vec4(lighting, 1.0f);
}
