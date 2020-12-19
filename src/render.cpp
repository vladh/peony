const char* render_pass_to_string(RenderPass::Flag render_pass) {
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
  } else {
    log_error("Don't know how to convert RenderPass to string: %d", render_pass);
    return "<unknown>";
  }
}


RenderPass::Flag render_pass_from_string(const char* str) {
  if (strcmp(str, "none") == 0) {
    return RenderPass::none;
  } else if (strcmp(str, "shadowcaster")) {
    return RenderPass::shadowcaster;
  } else if (strcmp(str, "deferred")) {
    return RenderPass::deferred;
  } else if (strcmp(str, "forward_depth")) {
    return RenderPass::forward_depth;
  } else if (strcmp(str, "forward_nodepth")) {
    return RenderPass::forward_nodepth;
  } else if (strcmp(str, "forward_skybox")) {
    return RenderPass::forward_skybox;
  } else if (strcmp(str, "lighting")) {
    return RenderPass::lighting;
  } else if (strcmp(str, "postprocessing")) {
    return RenderPass::postprocessing;
  } else if (strcmp(str, "preblur")) {
    return RenderPass::preblur;
  } else if (strcmp(str, "blur1")) {
    return RenderPass::blur1;
  } else if (strcmp(str, "blur2")) {
    return RenderPass::blur2;
  } else {
    log_fatal("Could not parse RenderPass: %s", str);
    return RenderPass::none;
  }
}
