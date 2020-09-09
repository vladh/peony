#version 330 core

uniform float t;
uniform vec3 light_position;
uniform vec3 camera_position;

in vec3 vertex_normal;
in vec3 frag_position;

out vec4 frag_color;

void main() {
  vec3 object_color = vec3(1.0f, 0.0f, 0.0f);
  vec3 light_color = vec3(1.0f, 1.0f, 1.0f);

  vec3 unit_normal = normalize(vertex_normal);
  vec3 light_dir = normalize(light_position - frag_position);

  float ambient_intensity = 0.1f;
  vec3 ambient = ambient_intensity * light_color;

  float diffuse_intensity = max(dot(unit_normal, light_dir), 0.0f);
  vec3 diffuse = diffuse_intensity * light_color;

  vec3 view_dir = normalize(camera_position - frag_position);
  vec3 reflect_dir = reflect(-light_dir, unit_normal);

  float specular_factor = 0.5;
  float specular_intensity = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
  vec3 specular = specular_factor * specular_intensity * light_color;

  vec3 light = (ambient + diffuse + specular) * object_color;
  /* vec3 light = (specular) * object_color; */

  frag_color = vec4(light, 1.0f);
}
