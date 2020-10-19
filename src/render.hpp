constexpr uint16 MAX_N_LIGHTS = 8;

enum RenderMode {
  RENDERMODE_REGULAR,
  RENDERMODE_DEPTH
};

enum RenderPass {
  RENDERPASS_DEFERRED,
  RENDERPASS_FORWARD_DEPTH,
  RENDERPASS_FORWARD_NODEPTH,
  RENDERPASS_LIGHTING
};

enum LightType {
  LIGHT_POINT,
  LIGHT_DIRECTIONAL
};

struct ShaderCommon {
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 shadow_transforms[6 * MAX_N_LIGHTS];

  glm::vec3 camera_position;
  float pad_oops;

  float exposure;
  float t;
  float far_clip_dist;
  int n_lights;

  int shadow_light_idx;
  float pad_oops1;
  float pad_oops2;
  float pad_oops3;

  glm::vec4 light_position[MAX_N_LIGHTS];
  glm::vec4 light_type[MAX_N_LIGHTS];
  glm::vec4 light_direction[MAX_N_LIGHTS];
  glm::vec4 light_color[MAX_N_LIGHTS];
  glm::vec4 light_attenuation[MAX_N_LIGHTS];
};
