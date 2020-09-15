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

uniform sampler2D texture_diffuse1;
uniform Light lights[8];
uniform int n_lights;
uniform int n_textures;
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

void main() {
  vec3 unit_normal = normalize(fs_in.normal);
  vec3 lighting = vec3(0.0f, 0.0f, 0.0f);

  for (int idx = 0; idx < n_lights; idx++) {
    Light light = lights[idx];

    vec3 entity_surface;
    if (n_textures > 0) {
      entity_surface = texture(texture_diffuse1, fs_in.tex_coords).rgb;
    } else {
      entity_surface = entity_color;
    }

    // Ambient
    vec3 ambient = light.ambient * entity_surface;

    // Diffuse
    vec3 light_dir = normalize(light.position - fs_in.frag_position);
    float diffuse_intensity = max(dot(unit_normal, light_dir), 0.0f);
    vec3 diffuse = light.diffuse * diffuse_intensity * entity_surface;

    // Specular
    vec3 view_dir = normalize(camera_position - fs_in.frag_position);
    vec3 reflect_dir = reflect(-light_dir, unit_normal);
    float specular_intensity = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = light.specular * specular_intensity * entity_surface;

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

    lighting += ambient + diffuse + specular;
  }

  if (should_draw_normals) {
    frag_color = vec4(fs_in.normal, 1.0f);
  } else {
    frag_color = vec4(lighting, 1.0f);
  }
}
