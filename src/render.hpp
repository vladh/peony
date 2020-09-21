#define MAX_N_LIGHTS 8
#define MAX_N_SHADOW_FRAMEBUFFERS MAX_N_LIGHTS
#define MAX_N_TEXTURES 8

enum RenderMode {
  RENDERMODE_REGULAR, RENDERMODE_DEPTH
};

struct Light {
  glm::vec3 position;
  float pad_3;
  glm::vec3 direction;
  float pad_7;
  glm::vec3 ambient;
  float pad_11;
  glm::vec3 diffuse;
  float pad_15;
  glm::vec3 specular;
  float pad_19;
  real32 attenuation_constant;
  real32 attenuation_linear;
  real32 attenuation_quadratic;
  float pad_23;
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
