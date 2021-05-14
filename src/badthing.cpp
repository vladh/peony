namespace badthing {
  // -----------------------------------------------------------
  // Types
  // -----------------------------------------------------------
  enum class RenderMode {regular, depth};

  typedef uint32 RenderPassFlag;

  namespace RenderPass {
    RenderPassFlag none = 0;
    RenderPassFlag shadowcaster = (1 << 0);
    RenderPassFlag deferred = (1 << 1);
    RenderPassFlag forward_depth = (1 << 2);
    RenderPassFlag forward_nodepth = (1 << 3);
    RenderPassFlag forward_skybox = (1 << 4);
    RenderPassFlag lighting = (1 << 5);
    RenderPassFlag postprocessing = (1 << 6);
    RenderPassFlag preblur = (1 << 7);
    RenderPassFlag blur1 = (1 << 8);
    RenderPassFlag blur2 = (1 << 9);
    RenderPassFlag renderdebug = (1 << 10);
  };

  struct ShaderCommon {
    m4 view;
    m4 projection;
    m4 ui_projection;
    m4 shadowmap_3d_transforms[6 * MAX_N_LIGHTS];
    m4 shadowmap_2d_transforms[MAX_N_LIGHTS];

    v3 camera_position;
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
    TextureType renderdebug_displayed_texture_type;
    int unused_pad;

    float exposure;
    float t;
    int window_width;
    int window_height;

    v4 point_light_position[MAX_N_LIGHTS];
    v4 point_light_color[MAX_N_LIGHTS];
    v4 point_light_attenuation[MAX_N_LIGHTS];

    v4 directional_light_position[MAX_N_LIGHTS];
    v4 directional_light_direction[MAX_N_LIGHTS];
    v4 directional_light_color[MAX_N_LIGHTS];
    v4 directional_light_attenuation[MAX_N_LIGHTS];
  };


  // -----------------------------------------------------------
  // Private functions
  // -----------------------------------------------------------
  const char* render_pass_to_string(RenderPassFlag render_pass) {
    if (render_pass == RenderPass::none) {
      return "none";
    } else if (render_pass == RenderPass::shadowcaster) {
      return "shadowcaster";
    } else if (render_pass == RenderPass::deferred) {
      return "deferred";
    } else if (render_pass == RenderPass::forward_depth) {
      return "forward_depth";
    } else if (render_pass == RenderPass::forward_nodepth) {
      return "forward_nodepth";
    } else if (render_pass == RenderPass::forward_skybox) {
      return "forward_skybox";
    } else if (render_pass == RenderPass::lighting) {
      return "lighting";
    } else if (render_pass == RenderPass::postprocessing) {
      return "postprocessing";
    } else if (render_pass == RenderPass::preblur) {
      return "preblur";
    } else if (render_pass == RenderPass::blur1) {
      return "blur1";
    } else if (render_pass == RenderPass::blur2) {
      return "blur2";
    } else if (render_pass == RenderPass::renderdebug) {
      return "renderdebug";
    } else {
      logs::error("Don't know how to convert RenderPass to string: %d", render_pass);
      return "<unknown>";
    }
  }


  RenderPassFlag render_pass_from_string(const char* str) {
    if (str::eq(str, "none")) {
      return RenderPass::none;
    } else if (str::eq(str, "shadowcaster")) {
      return RenderPass::shadowcaster;
    } else if (str::eq(str, "deferred")) {
      return RenderPass::deferred;
    } else if (str::eq(str, "forward_depth")) {
      return RenderPass::forward_depth;
    } else if (str::eq(str, "forward_nodepth")) {
      return RenderPass::forward_nodepth;
    } else if (str::eq(str, "forward_skybox")) {
      return RenderPass::forward_skybox;
    } else if (str::eq(str, "lighting")) {
      return RenderPass::lighting;
    } else if (str::eq(str, "postprocessing")) {
      return RenderPass::postprocessing;
    } else if (str::eq(str, "preblur")) {
      return RenderPass::preblur;
    } else if (str::eq(str, "blur1")) {
      return RenderPass::blur1;
    } else if (str::eq(str, "blur2")) {
      return RenderPass::blur2;
    } else if (str::eq(str, "renderdebug")) {
      return RenderPass::renderdebug;
    } else {
      logs::fatal("Could not parse RenderPass: %s", str);
      return RenderPass::none;
    }
  }


  // -----------------------------------------------------------
  // Public functions
  // -----------------------------------------------------------
}

using badthing::RenderMode, badthing::RenderPassFlag, badthing::ShaderCommon;
namespace RenderPass = badthing::RenderPass;
