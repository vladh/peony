#define MAX_N_LIGHTS 8
#define MAX_N_SHADOW_FRAMEBUFFERS MAX_N_LIGHTS
#define MAX_N_TEXTURES 8

enum RenderMode {
  RENDERMODE_REGULAR, RENDERMODE_DEPTH
};

struct Light {
  glm::vec3 position;
  real32 pad_position;
  glm::vec3 direction;
  real32 pad_direction;
  glm::vec3 ambient;
  real32 pad_ambient;
  glm::vec3 diffuse;
  real32 pad_diffuse;
  glm::vec3 specular;
  real32 pad_specular;
  glm::vec4 color;
  real32 attenuation_constant;
  real32 attenuation_linear;
  real32 attenuation_quadratic;
  real32 pad_attenuation;
};

struct ShaderCommon {
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 shadow_transforms[6];
  glm::vec3 camera_position;
  float pad_15;
  float t;
  float far_clip_dist;
  int n_lights;
  float pad_19;
  Light lights[MAX_N_LIGHTS];
};
