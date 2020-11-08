constexpr uint16 MAX_N_LIGHTS = 8;
constexpr real32 SHADOWMAP_ORTHO_PROJECTION_SIZE_FACTOR = 100.0f;
constexpr real32 TEXTURE_SHADOWMAP_SCREEN_SIZE_FACTOR = 3.0f;

// The position is used in positioning the shadow map, but not
// in the light calculations. We need to scale the position
// by some factor such that the shadow map covers the biggest
// possible area.
constexpr glm::vec3 DIRECTIONAL_LIGHT_DISTANCE = glm::vec3(
  75.0f, 15.0f, 75.0f
);

enum RenderMode {
  RENDERMODE_REGULAR,
  RENDERMODE_DEPTH
};

enum RenderPass {
  RENDERPASS_DEFERRED,
  RENDERPASS_FORWARD_DEPTH,
  RENDERPASS_FORWARD_NODEPTH,
  RENDERPASS_FORWARD_SKYBOX,
  RENDERPASS_LIGHTING,
  RENDERPASS_POSTPROCESSING,
  RENDERPASS_PREBLUR,
  RENDERPASS_BLUR1,
  RENDERPASS_BLUR2
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
  float camera_near_clip_dist;
  float camera_far_clip_dist;

  int n_lights;
  int shadow_light_idx;
  float shadow_far_clip_dist;
  bool is_blur_horizontal;

  float exposure;
  float t;
  int window_width;
  int window_height;

  glm::vec4 light_position[MAX_N_LIGHTS];
  glm::vec4 light_type[MAX_N_LIGHTS];
  glm::vec4 light_direction[MAX_N_LIGHTS];
  glm::vec4 light_color[MAX_N_LIGHTS];
  glm::vec4 light_attenuation[MAX_N_LIGHTS];
};
