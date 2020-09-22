#define MAX_N_LIGHTS 8
#define MAX_N_SHADOW_FRAMEBUFFERS MAX_N_LIGHTS
#define MAX_N_TEXTURES 8

enum RenderMode {
  RENDERMODE_REGULAR, RENDERMODE_DEPTH
};

struct Light {
  glm::vec4 position;
  glm::vec4 direction;
  glm::vec4 color;
  glm::vec4 attenuation;
};

struct ShaderCommon {
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 shadow_transforms[6];
  glm::vec3 camera_position;
  float pad_camera;
  float t;
  float far_clip_dist;
  int n_lights;
  float pad_floats;
  Light lights[MAX_N_LIGHTS];
};
