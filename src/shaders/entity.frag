#version 330 core

#define MAX_N_LIGHTS 32
#define MAX_N_SHADOW_FRAMEBUFFERS MAX_N_LIGHTS

struct Light {
  vec3 position;
  float pad_3;
  vec3 direction;
  float pad_7;
  vec3 ambient;
  float pad_11;
  vec3 diffuse;
  float pad_15;
  vec3 specular;
  float pad_19;
  float attenuation_constant;
  float attenuation_linear;
  float attenuation_quadratic;
  float pad_23;
};

layout (std140) uniform shader_common {
  mat4 view;
  mat4 projection;
  mat4 shadow_transforms[6];
  vec3 camera_position;
  float pad_15;
  float t;
  float far_clip_dist;
  int n_lights;
  float pad_19;
  Light lights[MAX_N_LIGHTS];
};

uniform int n_diffuse_textures;
uniform sampler2D diffuse_textures[32];
uniform int n_specular_textures;
uniform sampler2D specular_textures[32];
uniform int n_depth_textures;
uniform samplerCube depth_textures[MAX_N_SHADOW_FRAMEBUFFERS];

uniform vec3 entity_color;
uniform bool should_draw_normals;

in VS_OUT {
  vec3 normal;
  vec2 tex_coords;
  vec3 frag_position;
} fs_in;

out vec4 frag_color;

vec3 grid_sampling_offsets[20] = vec3[] (
  vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1),
  vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
  vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
  vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
  vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float calculate_shadows(vec3 frag_position, int idx_light) {
  vec3 frag_to_light = fs_in.frag_position - lights[idx_light].position;
  float current_depth = length(frag_to_light);

  float shadow = 0.0f;
  float bias = 0.30f;
  int n_samples = 20;

  float view_distance = length(camera_position - fs_in.frag_position);
  float sample_radius = (1.0f + (view_distance / far_clip_dist)) / 25.0f;

  for (int i = 0; i < n_samples; i++) {
    float closest_depth = texture(
      /* depth_textures[idx_light], */
      depth_textures[0],
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
  if (should_draw_normals) {
    frag_color = vec4(fs_in.normal, 1.0f);
    return;
  }

  vec3 unit_normal = normalize(fs_in.normal);
  vec3 lighting = vec3(0.0f, 0.0f, 0.0f);

  for (int idx_light = 0; idx_light < n_lights; idx_light++) {
    Light light = lights[idx_light];

    vec3 entity_surface;
    if (n_diffuse_textures > 0) {
      entity_surface = texture(diffuse_textures[0], fs_in.tex_coords).rgb;
    } else {
      entity_surface = entity_color;
    }

    // Ambient
    vec3 ambient = light.ambient;

    // Diffuse
    vec3 light_dir = normalize(light.position - fs_in.frag_position);
    float diffuse_intensity = max(dot(unit_normal, light_dir), 0.0f);
    vec3 diffuse = light.diffuse * diffuse_intensity;

    // Specular
    vec3 view_dir = normalize(camera_position - fs_in.frag_position);
    vec3 reflect_dir = reflect(-light_dir, unit_normal);
    float specular_intensity = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = light.specular * specular_intensity;

    // Attenuation
    float distance = length(light.position - fs_in.frag_position);
    float attenuation = 1.0 / (
      light.attenuation_constant +
      light.attenuation_linear * distance +
      light.attenuation_quadratic * (distance * distance)
    );
    diffuse *= attenuation;
    specular *= attenuation;

    if (n_depth_textures >= n_lights) {
      float shadow = calculate_shadows(fs_in.frag_position, idx_light);
      lighting += (ambient + ((1.0f - shadow) * (diffuse + specular))) * entity_surface;
    } else {
      lighting += (ambient + diffuse + specular) * entity_surface;
    }
  }

  frag_color = vec4(lighting, 1.0f);
}
