enum RenderMode {
  RENDERMODE_REGULAR, RENDERMODE_DEPTH
};

struct Light {
  bool is_point_light;
  glm::vec3 position;
  glm::vec3 direction;
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  real32 attenuation_constant;
  real32 attenuation_linear;
  real32 attenuation_quadratic;
};
