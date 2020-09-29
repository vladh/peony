constexpr uint16 MAX_N_LIGHTS = 8;
constexpr uint16 MAX_N_SHADOW_FRAMEBUFFERS = MAX_N_LIGHTS;

enum RenderMode {
  RENDERMODE_REGULAR, RENDERMODE_DEPTH
};

enum RenderPass {
  RENDERPASS_DEFERRED, RENDERPASS_FORWARD, RENDERPASS_LIGHTING
};

struct Light {
  glm::vec4 position;
  glm::vec4 color;
  glm::vec4 attenuation;
};

struct ShaderCommon {
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 shadow_transforms[6];

  glm::vec3 camera_position;
  float pad_woops;

  float exposure;
  float t;
  float far_clip_dist;
  int n_lights;

  glm::vec4 light_position[MAX_N_LIGHTS];
  glm::vec4 light_color[MAX_N_LIGHTS];
  glm::vec4 light_attenuation[MAX_N_LIGHTS];
};
