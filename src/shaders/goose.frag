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

uniform Light lights[8];
uniform int n_lights;
uniform float t;
uniform vec3 camera_position;

in vec3 vertex_normal;
in vec3 frag_position;

out vec4 frag_color;


void main() {
  vec3 object_color = vec3(1.0f, 0.0f, 0.0f);
  vec3 unit_normal = normalize(vertex_normal);
  vec3 lighting = vec3(0.0f, 0.0f, 0.0f);

  for (int idx = 0; idx < n_lights; idx++) {
    Light light = lights[idx];

    // Ambient
    vec3 ambient = light.ambient * object_color;

    // Diffuse
    vec3 light_dir = normalize(light.position - frag_position);
    float diffuse_intensity = max(dot(unit_normal, light_dir), 0.0f);
    vec3 diffuse = light.diffuse * diffuse_intensity * object_color;

    // Specular
    vec3 view_dir = normalize(camera_position - frag_position);
    vec3 reflect_dir = reflect(-light_dir, unit_normal);
    float specular_intensity = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
    vec3 specular = light.specular * specular_intensity * object_color;

    // Attenuation
    float distance = length(light.position - frag_position);
    float attenuation = 1.0 / (
      light.attenuation_constant +
      light.attenuation_linear * distance +
      light.attenuation_quadratic * (distance * distance)
    );
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;

    lighting += ambient + diffuse + specular;
  }

  frag_color = vec4(lighting, 1.0f);
}
