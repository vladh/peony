#version 330 core

uniform float t;
uniform vec3 light_position;

in vec3 vertex_normal;
in vec3 frag_position;

out vec4 frag_color;

void main() {
  vec3 object_color = vec3(1.0f, 1.0f, 1.0f);

  vec3 light_color = vec3(1.0f, 0.0f, 0.0f);

  float ambient_intensity = 0.1f;
  vec3 ambient = ambient_intensity * light_color;

  vec3 unit_normal = normalize(vertex_normal);
  vec3 light_dir = normalize(light_position - frag_position);
  float diffuse_intensity = max(dot(unit_normal, light_dir), 0.0f);
  vec3 diffuse = diffuse_intensity * light_color;

  vec3 light = (ambient + diffuse) * object_color;

  frag_color = vec4(light, 1.0f);
}
