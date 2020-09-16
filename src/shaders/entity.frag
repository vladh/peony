#version 330 core

struct Light {
  bool is_point_light;
  vec3 position;
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float attenuation_constant;
  float attenuation_linear;
  float attenuation_quadratic;
};

uniform int n_diffuse_textures;
uniform sampler2D diffuse_textures[16];
uniform int n_specular_textures;
uniform sampler2D specular_textures[16];
uniform int n_depth_textures;
uniform samplerCube depth_textures[16];

uniform vec3 depth_light_position;
uniform float far_clip_dist;

uniform Light lights[8];
uniform int n_lights;

uniform float t;
uniform vec3 camera_position;
uniform vec3 entity_color;
uniform bool should_draw_normals;

in VS_OUT {
  vec3 normal;
  vec2 tex_coords;
  vec3 frag_position;
} fs_in;

out vec4 frag_color;

vec3 grid_sampling_offsets[20] = vec3[] (
  vec3(1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1),
  vec3(1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
  vec3(1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
  vec3(1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
  vec3(0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
);

float calculate_shadows(vec3 frag_position) {
  vec3 frag_to_light = fs_in.frag_position - depth_light_position;
  float current_depth = length(frag_to_light);

  float shadow = 0.0f;
  float bias = 0.30f;
  int n_samples = 20;

  float view_distance = length(camera_position - fs_in.frag_position);
  float sample_radius = (1.0f + (view_distance / far_clip_dist)) / 25.0f;

  for (int i = 0; i < n_samples; i++) {
    float closest_depth = texture(
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
  vec3 unit_normal = normalize(fs_in.normal);
  vec3 lighting = vec3(0.0f, 0.0f, 0.0f);

  for (int idx = 0; idx < n_lights; idx++) {
    Light light = lights[idx];

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
    /* ambient *= attenuation; */
    diffuse *= attenuation;
    specular *= attenuation;

    if (n_depth_textures > 0) {
      float shadow = calculate_shadows(fs_in.frag_position);
      /* lighting += shadow; */
      lighting += (ambient + ((1.0f - shadow) * (diffuse + specular))) * entity_surface;
    } else {
      lighting += (ambient + diffuse + specular) * entity_surface;
    }
  }

  if (should_draw_normals) {
    frag_color = vec4(fs_in.normal, 1.0f);
  } else {
    frag_color = vec4(lighting, 1.0f);
  }
}
