constexpr uint16 MAX_N_LIGHTS = 8;
constexpr real32 DIRECTIONAL_LIGHT_DISTANCE = 15.0f;
constexpr real32 SHADOWMAP_ORTHO_PROJECTION_SIZE_FACTOR = 200.0f;
constexpr real32 TEXTURE_SHADOWMAP_SCREEN_SIZE_FACTOR = 4.0f;

enum RenderMode {
  RENDERMODE_REGULAR,
  RENDERMODE_DEPTH
};

enum RenderPass {
  RENDERPASS_DEFERRED,
  RENDERPASS_FORWARD_DEPTH,
  RENDERPASS_FORWARD_NODEPTH,
  RENDERPASS_FORWARD_SKYBOX,
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
  float camera_pitch;

  float camera_horizontal_fov;
  float camera_vertical_fov;
  int shadow_light_idx;
  float pad_oops1;

  float exposure;
  float t;
  float far_clip_dist;
  int n_lights;

  glm::vec4 light_position[MAX_N_LIGHTS];
  glm::vec4 light_type[MAX_N_LIGHTS];
  glm::vec4 light_direction[MAX_N_LIGHTS];
  glm::vec4 light_color[MAX_N_LIGHTS];
  glm::vec4 light_attenuation[MAX_N_LIGHTS];
};
