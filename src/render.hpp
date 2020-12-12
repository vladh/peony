constexpr uint16 MAX_N_LIGHTS = 8;

// The position is used in positioning the shadow map, but not
// in the light calculations. We need to scale the position
// by some factor such that the shadow map covers the biggest
// possible area.
constexpr glm::vec3 DIRECTIONAL_LIGHT_DISTANCE = glm::vec3(
  75.0f, 15.0f, 75.0f
);

enum class RenderMode {regular, depth};

/* enum class RenderPass { */
/*   deferred, */
/*   forward_depth, */
/*   forward_nodepth, */
/*   forward_skybox, */
/*   lighting, */
/*   postprocessing, */
/*   preblur, */
/*   blur1, */
/*   blur2 */
/* }; */

namespace RenderPass {
  typedef uint32 Flag;
  Flag none = 0;
  Flag shadowcaster = (1 << 0);
  Flag deferred = (1 << 1);
  Flag forward_depth = (1 << 2);
  Flag forward_nodepth = (1 << 3);
  Flag forward_skybox = (1 << 4);
  Flag lighting = (1 << 5);
  Flag postprocessing = (1 << 6);
  Flag preblur = (1 << 7);
  Flag blur1 = (1 << 8);
  Flag blur2 = (1 << 9);
};

struct ShaderCommon {
  glm::mat4 view;
  glm::mat4 projection;
  glm::mat4 ui_projection;
  glm::mat4 cube_shadowmap_transforms[6 * MAX_N_LIGHTS];
  glm::mat4 texture_shadowmap_transforms[MAX_N_LIGHTS];

  glm::vec3 camera_position;
  float camera_pitch;

  float camera_horizontal_fov;
  float camera_vertical_fov;
  float camera_near_clip_dist;
  float camera_far_clip_dist;

  int n_point_lights;
  int n_directional_lights;
  int current_shadow_light_idx;
  int current_shadow_light_type;

  float shadow_far_clip_dist;
  bool is_blur_horizontal;
  float pad_1;
  float pad_2;

  float exposure;
  float t;
  int window_width;
  int window_height;

  glm::vec4 point_light_position[MAX_N_LIGHTS];
  glm::vec4 point_light_color[MAX_N_LIGHTS];
  glm::vec4 point_light_attenuation[MAX_N_LIGHTS];

  glm::vec4 directional_light_position[MAX_N_LIGHTS];
  glm::vec4 directional_light_direction[MAX_N_LIGHTS];
  glm::vec4 directional_light_color[MAX_N_LIGHTS];
  glm::vec4 directional_light_attenuation[MAX_N_LIGHTS];
};
