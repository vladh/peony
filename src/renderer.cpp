namespace renderer {
  // -----------------------------------------------------------
  // Private functions
  // -----------------------------------------------------------
  void init_ubo(State *state) {
    glGenBuffers(1, &state->ubo_shader_common);
    glBindBuffer(GL_UNIFORM_BUFFER, state->ubo_shader_common);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderCommon), NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, state->ubo_shader_common, 0, sizeof(ShaderCommon));
  }


  void init_shadowmaps(
    MemoryPool *memory_pool,
    BuiltinTextures *builtin_textures
  ) {
    // Cube
    glGenFramebuffers(1, &builtin_textures->shadowmaps_3d_framebuffer);
    glGenTextures(1, &builtin_textures->shadowmaps_3d);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, builtin_textures->shadowmaps_3d);

    glTexStorage3D(
      GL_TEXTURE_CUBE_MAP_ARRAY, 1, GL_DEPTH_COMPONENT32F,
      builtin_textures->shadowmap_3d_width, builtin_textures->shadowmap_3d_height,
      6 * MAX_N_LIGHTS
    );

    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, builtin_textures->shadowmaps_3d_framebuffer);
    glFramebufferTexture(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, builtin_textures->shadowmaps_3d, 0
    );

    // #slow
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      logs::fatal("Framebuffer not complete!");
    }

    builtin_textures->shadowmaps_3d_texture = materials::init_texture(
      (Texture*)memory::push(
        memory_pool, sizeof(Texture), "shadowmaps_3d_texture"
      ),
      GL_TEXTURE_CUBE_MAP_ARRAY,
      TextureType::shadowmaps_3d, builtin_textures->shadowmaps_3d,
      builtin_textures->shadowmap_3d_width, builtin_textures->shadowmap_3d_height, 1
    );
    builtin_textures->shadowmaps_3d_texture->is_builtin = true;

    // Texture
    glGenFramebuffers(1, &builtin_textures->shadowmaps_2d_framebuffer);
    glGenTextures(1, &builtin_textures->shadowmaps_2d);
    glBindTexture(GL_TEXTURE_2D_ARRAY, builtin_textures->shadowmaps_2d);

    glTexStorage3D(
      GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32F,
      builtin_textures->shadowmap_2d_width,
      builtin_textures->shadowmap_2d_height,
      MAX_N_LIGHTS
    );

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    real32 border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border_color);
    glBindFramebuffer(GL_FRAMEBUFFER, builtin_textures->shadowmaps_2d_framebuffer);
    glFramebufferTexture(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, builtin_textures->shadowmaps_2d, 0
    );

    // #slow
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      logs::fatal("Framebuffer not complete!");
    }

    builtin_textures->shadowmaps_2d_texture = materials::init_texture(
      (Texture*)memory::push(
        memory_pool, sizeof(Texture), "shadowmaps_2d_texture"
      ),
      GL_TEXTURE_2D_ARRAY,
      TextureType::shadowmaps_2d, builtin_textures->shadowmaps_2d,
      builtin_textures->shadowmap_2d_width,
      builtin_textures->shadowmap_2d_height, 1
    );
    builtin_textures->shadowmaps_2d_texture->is_builtin = true;
  }


  void init_g_buffer(
    MemoryPool *memory_pool,
    BuiltinTextures *builtin_textures,
    uint32 width,
    uint32 height
  ) {
    glGenFramebuffers(1, &builtin_textures->g_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, builtin_textures->g_buffer);

    uint32 g_position_texture_name;
    uint32 g_normal_texture_name;
    uint32 g_albedo_texture_name;
    uint32 g_pbr_texture_name;

    glGenTextures(1, &g_position_texture_name);
    glGenTextures(1, &g_normal_texture_name);
    glGenTextures(1, &g_albedo_texture_name);
    glGenTextures(1, &g_pbr_texture_name);

    builtin_textures->g_position_texture = materials::init_texture(
      (Texture*)memory::push(
        memory_pool, sizeof(Texture), "g_position_texture"
      ),
      GL_TEXTURE_2D, TextureType::g_position, g_position_texture_name,
      width, height, 4
    );
    builtin_textures->g_position_texture->is_builtin = true;

    builtin_textures->g_normal_texture = materials::init_texture(
      (Texture*)memory::push(
        memory_pool, sizeof(Texture), "g_normal_texture"
      ),
      GL_TEXTURE_2D, TextureType::g_normal, g_normal_texture_name,
      width, height, 4
    );
    builtin_textures->g_normal_texture->is_builtin = true;

    builtin_textures->g_albedo_texture = materials::init_texture(
      (Texture*)memory::push(
        memory_pool, sizeof(Texture), "g_albedo_texture"
      ),
      GL_TEXTURE_2D, TextureType::g_albedo, g_albedo_texture_name,
      width, height, 4
    );
    builtin_textures->g_albedo_texture->is_builtin = true;

    builtin_textures->g_pbr_texture = materials::init_texture(
      (Texture*)memory::push(
        memory_pool, sizeof(Texture), "g_pbr_texture"
      ),
      GL_TEXTURE_2D, TextureType::g_pbr, g_pbr_texture_name,
      width, height, 4
    );
    builtin_textures->g_pbr_texture->is_builtin = true;

    glBindTexture(GL_TEXTURE_2D, builtin_textures->g_position_texture->texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F,
      builtin_textures->g_position_texture->width, builtin_textures->g_position_texture->height,
      0, GL_RGBA, GL_FLOAT, NULL
    );
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      builtin_textures->g_position_texture->texture_name, 0
    );

    glBindTexture(GL_TEXTURE_2D, builtin_textures->g_normal_texture->texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F,
      builtin_textures->g_normal_texture->width, builtin_textures->g_normal_texture->height,
      0, GL_RGBA, GL_FLOAT, NULL
    );
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
      builtin_textures->g_normal_texture->texture_name, 0
    );

    glBindTexture(GL_TEXTURE_2D, builtin_textures->g_albedo_texture->texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA,
      builtin_textures->g_albedo_texture->width, builtin_textures->g_albedo_texture->height,
      0, GL_RGBA, GL_UNSIGNED_BYTE, NULL
    );
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
      builtin_textures->g_albedo_texture->texture_name, 0
    );

    glBindTexture(GL_TEXTURE_2D, builtin_textures->g_pbr_texture->texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA,
      builtin_textures->g_pbr_texture->width, builtin_textures->g_pbr_texture->height,
      0, GL_RGBA, GL_UNSIGNED_BYTE, NULL
    );
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D,
      builtin_textures->g_pbr_texture->texture_name, 0
    );

    uint32 attachments[4] = {
      GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
      GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3
    };
    glDrawBuffers(4, attachments);

    uint32 rbo_depth;
    glGenRenderbuffers(1, &rbo_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
    glRenderbufferStorage(
      GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
      width, height
    );
    glFramebufferRenderbuffer(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth
    );

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      logs::fatal("Framebuffer not complete!");
    }
  }


  void init_l_buffer(
    MemoryPool *memory_pool,
    BuiltinTextures *builtin_textures,
    uint32 width,
    uint32 height
  ) {
    glGenFramebuffers(1, &builtin_textures->l_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, builtin_textures->l_buffer);

    uint32 l_color_texture_name;
    glGenTextures(1, &l_color_texture_name);

    builtin_textures->l_color_texture = materials::init_texture(
      (Texture*)memory::push(
        memory_pool, sizeof(Texture), "l_color_texture"
      ),
      GL_TEXTURE_2D, TextureType::l_color, l_color_texture_name,
      width, height, 4
    );
    builtin_textures->l_color_texture->is_builtin = true;

    glBindTexture(GL_TEXTURE_2D, builtin_textures->l_color_texture->texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F,
      builtin_textures->l_color_texture->width, builtin_textures->l_color_texture->height,
      0, GL_RGBA, GL_FLOAT, NULL
    );
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      builtin_textures->l_color_texture->texture_name, 0
    );

    uint32 l_bright_color_texture_name;
    glGenTextures(1, &l_bright_color_texture_name);

    builtin_textures->l_bright_color_texture = materials::init_texture(
      (Texture*)memory::push(
        memory_pool, sizeof(Texture), "l_bright_color_texture"
      ),
      GL_TEXTURE_2D, TextureType::l_bright_color, l_bright_color_texture_name,
      width, height, 4
    );
    builtin_textures->l_bright_color_texture->is_builtin = true;

    glBindTexture(GL_TEXTURE_2D, builtin_textures->l_bright_color_texture->texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F,
      builtin_textures->l_bright_color_texture->width,
      builtin_textures->l_bright_color_texture->height,
      0, GL_RGBA, GL_FLOAT, NULL
    );
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
      builtin_textures->l_bright_color_texture->texture_name, 0
    );

    uint32 attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

#if 0
    uint32 rbo_depth;
    glGenRenderbuffers(1, &rbo_depth);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
    glRenderbufferStorage(
      GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
      width, height
    );
    glFramebufferRenderbuffer(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth
    );
#else
    uint32 l_depth_texture_name;
    glGenTextures(1, &l_depth_texture_name);

    builtin_textures->l_depth_texture = materials::init_texture(
      (Texture*)memory::push(
        memory_pool, sizeof(Texture), "l_depth_texture"
      ),
      GL_TEXTURE_2D, TextureType::l_depth, l_depth_texture_name,
      width, height, 1
    );
    builtin_textures->l_depth_texture->is_builtin = true;

    glBindTexture(GL_TEXTURE_2D, builtin_textures->l_depth_texture->texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
      builtin_textures->l_depth_texture->width, builtin_textures->l_depth_texture->height,
      0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL
    );
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
      builtin_textures->l_depth_texture->texture_name, 0
    );
#endif

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      logs::fatal("Framebuffer not complete!");
    }
  }


  void init_blur_buffers(
    MemoryPool *memory_pool,
    BuiltinTextures *builtin_textures,
    uint32 width,
    uint32 height
  ) {
#if USE_BLOOM
    return;
#endif
    glGenFramebuffers(1, &builtin_textures->blur1_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, builtin_textures->blur1_buffer);
    uint32 blur1_texture_name;
    glGenTextures(1, &blur1_texture_name);

    builtin_textures->blur1_texture = materials::init_texture(
      (Texture*)memory::push(
        memory_pool, sizeof(Texture), "blur1_texture"
      ),
      GL_TEXTURE_2D, TextureType::blur1, blur1_texture_name,
      width, height, 4
    );
    builtin_textures->blur1_texture->is_builtin = true;

    glBindTexture(GL_TEXTURE_2D, builtin_textures->blur1_texture->texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F,
      builtin_textures->blur1_texture->width, builtin_textures->blur1_texture->height,
      0, GL_RGBA, GL_FLOAT, NULL
    );
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      builtin_textures->blur1_texture->texture_name, 0
    );

    glGenFramebuffers(1, &builtin_textures->blur2_buffer);
    glBindFramebuffer(GL_FRAMEBUFFER, builtin_textures->blur2_buffer);
    uint32 blur2_texture_name;
    glGenTextures(1, &blur2_texture_name);

    builtin_textures->blur2_texture = materials::init_texture(
      (Texture*)memory::push(
        memory_pool, sizeof(Texture), "blur2_texture"
      ),
      GL_TEXTURE_2D, TextureType::blur2, blur2_texture_name,
      width, height, 4
    );
    builtin_textures->blur2_texture->is_builtin = true;

    glBindTexture(GL_TEXTURE_2D, builtin_textures->blur2_texture->texture_name);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGBA16F,
      builtin_textures->blur2_texture->width, builtin_textures->blur2_texture->height,
      0, GL_RGBA, GL_FLOAT, NULL
    );
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
      builtin_textures->blur2_texture->texture_name, 0
    );

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      logs::fatal("Framebuffer not complete!");
    }
  }


  void resize_renderer_buffers(
    MemoryPool *memory_pool,
    Array<Material> *materials,
    BuiltinTextures *builtin_textures,
    uint32 width,
    uint32 height
  ) {
    // TODO: Only regenerate once we're done resizing, not for every little bit
    // of the resize.
    init_g_buffer(
      memory_pool, builtin_textures, width, height
    );
    init_l_buffer(
      memory_pool, builtin_textures, width, height
    );
    init_blur_buffers(
      memory_pool, builtin_textures, width, height
    );

    for_each (material, *materials) {
      if (material->n_textures > 0 && material->is_screensize_dependent) {
        for (uint32 idx_texture = 0; idx_texture < material->n_textures; idx_texture++) {
          Texture *texture = &material->textures[idx_texture];
          if (texture->type == TextureType::g_position) {
            material->textures[idx_texture] = *builtin_textures->g_position_texture;
          } else if (texture->type == TextureType::g_normal) {
            material->textures[idx_texture] = *builtin_textures->g_normal_texture;
          } else if (texture->type == TextureType::g_albedo) {
            material->textures[idx_texture] = *builtin_textures->g_albedo_texture;
          } else if (texture->type == TextureType::g_pbr) {
            material->textures[idx_texture] = *builtin_textures->g_pbr_texture;
          } else if (texture->type == TextureType::l_color) {
            material->textures[idx_texture] = *builtin_textures->l_color_texture;
          } else if (texture->type == TextureType::l_bright_color) {
            material->textures[idx_texture] = *builtin_textures->l_bright_color_texture;
          } else if (texture->type == TextureType::l_depth) {
            material->textures[idx_texture] = *builtin_textures->l_depth_texture;
          } else if (texture->type == TextureType::blur1) {
            material->textures[idx_texture] = *builtin_textures->blur1_texture;
          } else if (texture->type == TextureType::blur2) {
            material->textures[idx_texture] = *builtin_textures->blur2_texture;
          }
        }
        materials::bind_texture_uniforms(material);
      }
    }
  }


  void update_drawing_options(State *state, GLFWwindow *window) {
    if (state->is_cursor_enabled) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    } else {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }

    if (state->should_use_wireframe) {
      // This will be handled in the rendering loop.
    } else {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  }


  void copy_scene_data_to_ubo(
    State *state,
    uint32 current_shadow_light_idx,
    uint32 current_shadow_light_type,
    bool32 is_blur_horizontal
  ) {
    ShaderCommon *shader_common = &state->shader_common;

    shader_common->view = state->camera_active->view;
    shader_common->projection = state->camera_active->projection;
    shader_common->ui_projection = state->camera_active->ui_projection;
    memcpy(
      shader_common->shadowmap_3d_transforms,
      state->shadowmap_3d_transforms,
      sizeof(state->shadowmap_3d_transforms)
    );
    memcpy(
      shader_common->shadowmap_2d_transforms,
      state->shadowmap_2d_transforms,
      sizeof(state->shadowmap_2d_transforms)
    );

    shader_common->camera_position = v4(state->camera_active->position, 1.0f);
    shader_common->camera_pitch = (float)state->camera_active->pitch;

    shader_common->camera_horizontal_fov = state->camera_active->horizontal_fov;
    shader_common->camera_vertical_fov = state->camera_active->vertical_fov;
    shader_common->camera_near_clip_dist = state->camera_active->near_clip_dist;
    shader_common->camera_far_clip_dist = state->camera_active->far_clip_dist;

    shader_common->current_shadow_light_idx = current_shadow_light_idx;
    shader_common->current_shadow_light_type = current_shadow_light_type;

    shader_common->shadow_far_clip_dist = state->builtin_textures.shadowmap_far_clip_dist;
    shader_common->is_blur_horizontal = is_blur_horizontal;
    shader_common->renderdebug_displayed_texture_type =
      state->renderdebug_displayed_texture_type;
    shader_common->unused_pad = 0;

    shader_common->exposure = state->camera_active->exposure;
    shader_common->t = (float)state->t;
    shader_common->window_width = state->window_info.width;
    shader_common->window_height = state->window_info.height;

    uint32 n_point_lights = 0;
    uint32 n_directional_lights = 0;

    for_each (light_component, state->light_component_set.components) {
      if (light_component->entity_handle == Entity::no_entity_handle) {
        continue;
      }

      SpatialComponent *spatial_component =
        state->spatial_component_set.components[light_component->entity_handle];

      if (!(
        lights::is_light_component_valid(light_component) &&
        spatial::is_spatial_component_valid(spatial_component)
      )) {
        continue;
      }

      if (light_component->type == LightType::point) {
        shader_common->point_light_position[n_point_lights] = v4(
          spatial_component->position, 1.0f
        );
        shader_common->point_light_color[n_point_lights] =
          light_component->color;
        shader_common->point_light_attenuation[n_point_lights] =
          light_component->attenuation;
        n_point_lights++;
      } else if (light_component->type == LightType::directional) {
        shader_common->directional_light_position[n_directional_lights] =
          v4(spatial_component->position, 1.0f);
        shader_common->directional_light_direction[n_directional_lights] =
          v4(light_component->direction, 1.0f);
        shader_common->directional_light_color[n_directional_lights] =
          light_component->color;
        shader_common->directional_light_attenuation[n_directional_lights] =
          light_component->attenuation;
        n_directional_lights++;
      }
    }

    shader_common->n_point_lights = n_point_lights;
    shader_common->n_directional_lights = n_directional_lights;

    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderCommon), shader_common);
  }


  void copy_scene_data_to_ubo(State *state) {
    copy_scene_data_to_ubo(state, 0, 0, false);
  }


  void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
    State *state = memory_and_state->state;
    MemoryPool *asset_memory_pool = memory_and_state->asset_memory_pool;
    logs::info(
      "Window is now: %d x %d", state->window_info.width, state->window_info.height
    );
    state->window_info.width = width;
    state->window_info.height = height;
    cameras::update_matrices(
      state->camera_active,
      state->window_info.width,
      state->window_info.height
    );
    cameras::update_ui_matrices(
      state->camera_active,
      state->window_info.width,
      state->window_info.height
    );
    gui::update_screen_dimensions(
      &state->gui_state, state->window_info.width, state->window_info.height
    );
    resize_renderer_buffers(
      asset_memory_pool,
      &state->materials,
      &state->builtin_textures,
      width,
      height
    );
  }


  void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
    State *state = memory_and_state->state;

    input::update_mouse_button(&state->input_state, button, action, mods);
    gui::update_mouse_button(&state->gui_state);
  }


  void mouse_callback(GLFWwindow *window, real64 x, real64 y) {
    MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
    State *state = memory_and_state->state;

    v2 mouse_pos = v2(x, y);
    input::update_mouse(&state->input_state, mouse_pos);

    if (state->is_cursor_enabled) {
      gui::update_mouse(&state->gui_state);
    } else {
      cameras::update_mouse(
        state->camera_active,
        state->input_state.mouse_3d_offset
      );
    }
  }


  void key_callback(
    GLFWwindow* window,
    int key, int scancode, int action, int mods
  ) {
    MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
    State *state = memory_and_state->state;
    input::update_keys(&state->input_state, key, scancode, action, mods);
  }


  void char_callback(
    GLFWwindow* window, uint32 codepoint
  ) {
    MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
    State *state = memory_and_state->state;
    input::update_text_input(&state->input_state, codepoint);
  }


  WindowInfo init_window() {
    WindowInfo window_info = {};
    strcpy(window_info.title, "hi lol");

    glfwInit();

    logs::info("Using OpenGL 4.1");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

#if defined(__APPLE__)
    logs::info("Using GLFW_OPENGL_FORWARD_COMPAT");
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    if (USE_OPENGL_DEBUG) {
      logs::info("Using OpenGL debug context");
      glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    }

    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);

    // Create the window. Right now we're working with screencoord sizes,
    // not pixels!

#if USE_FULLSCREEN
    int32 n_monitors;
    GLFWmonitor **monitors = glfwGetMonitors(&n_monitors);
    GLFWmonitor *target_monitor = monitors[TARGET_MONITOR];

    const GLFWvidmode *video_mode = glfwGetVideoMode(target_monitor);
    glfwWindowHint(GLFW_RED_BITS, video_mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, video_mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, video_mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, video_mode->refreshRate);

    window_info.screencoord_width = video_mode->width;
    window_info.screencoord_height = video_mode->height;

    GLFWwindow *window = glfwCreateWindow(
      window_info.screencoord_width, window_info.screencoord_height,
      window_info.title,
#if USE_WINDOWED_FULLSCREEN
      nullptr, nullptr
#else
      target_monitor, nullptr
#endif
    );
#else
    window_info.screencoord_width = 1920;
    window_info.screencoord_height = 1080;

    GLFWwindow *window = glfwCreateWindow(
      window_info.screencoord_width, window_info.screencoord_height,
      window_info.title,
      nullptr, nullptr
    );

    glfwSetWindowPos(window, 200, 200);
#endif

    if (!window) {
      logs::fatal("Failed to create GLFW window");
      return window_info;
    }
    window_info.window = window;

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      logs::fatal("Failed to initialize GLAD");
      return window_info;
    }

    if (!GLAD_GL_ARB_texture_cube_map_array) {
      logs::fatal("No support for GLAD_GL_ARB_texture_cube_map_array");
    }
    if (!GLAD_GL_ARB_texture_storage) {
      logs::fatal("No support for GLAD_GL_ARB_texture_storage");
    }
    if (!GLAD_GL_ARB_buffer_storage) {
      logs::warning("No support for GLAD_GL_ARB_buffer_storage");
    }

    // TODO: Remove GL_EXT_debug_marker from GLAD
    // TODO: Remove GL_EXT_debug_label from GLAD
    // TODO: Remove GL_ARB_texture_storage_multisample from GLAD

    if (USE_OPENGL_DEBUG) {
      if (GLAD_GL_AMD_debug_output || GLAD_GL_ARB_debug_output || GLAD_GL_KHR_debug) {
        GLint flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
          glEnable(GL_DEBUG_OUTPUT);
          glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
          glDebugMessageCallback(util::debug_message_callback, nullptr);
          glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        } else {
          logs::fatal("Tried to initialise OpenGL debug output but couldn't");
        }
      } else {
        logs::warning(
          "Tried to initialise OpenGL debug output but none of "
          "[GL_AMD_debug_output, GL_ARB_debug_output, GL_KHR_debug] "
          "are supported on this system. Skipping."
        );
      }
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glLineWidth(2.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Get the framebuffer size. This is the actual window size in pixels.
    int32 framebuffer_width;
    int32 framebuffer_height;
    glfwGetFramebufferSize(window, &framebuffer_width, &framebuffer_height);
    window_info.width = (uint32)framebuffer_width;
    window_info.height = (uint32)framebuffer_height;
    glViewport(0, 0, window_info.width, window_info.height);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);

    return window_info;
  }


  void destroy_window() {
    glfwTerminate();
  }


  void reload_shaders(State *state) {
    MemoryPool temp_memory_pool = {};

    for_each (material, state->materials) {
      shaders::load_shader_asset(
        &material->shader_asset,
        &temp_memory_pool
      );
      if (shaders::is_shader_asset_valid(&material->depth_shader_asset)) {
        shaders::load_shader_asset(
          &material->depth_shader_asset,
          &temp_memory_pool
        );
      }
    }

    shaders::load_shader_asset(
      &state->standard_depth_shader_asset,
      &temp_memory_pool
    );

    memory::destroy_memory_pool(&temp_memory_pool);
  }


  void draw(
    RenderMode render_mode,
    DrawableComponentSet *drawable_component_set,
    DrawableComponent *drawable_component,
    Material *material,
    m4 *model_matrix,
    m3 *model_normal_matrix,
    m4 *bone_matrices,
    ShaderAsset *standard_depth_shader_asset
  ) {
    ShaderAsset *shader_asset = nullptr;

    if (render_mode == RenderMode::regular) {
      shader_asset = &material->shader_asset;
    } else if (render_mode == RenderMode::depth) {
      if (shaders::is_shader_asset_valid(&material->depth_shader_asset)) {
        shader_asset = &material->depth_shader_asset;
      } else {
        shader_asset = standard_depth_shader_asset;
      }
    }

    assert(shader_asset);

    // If our shader program has changed since our last mesh, tell OpenGL about it.
    if (shader_asset->program != drawable_component_set->last_drawn_shader_program) {
      glUseProgram(shader_asset->program);
      drawable_component_set->last_drawn_shader_program = shader_asset->program;

      if (render_mode == RenderMode::regular) {
        for (
          uint32 texture_idx = 1;
          texture_idx < shader_asset->n_texture_units + 1; texture_idx++
        ) {
          if (shader_asset->texture_units[texture_idx] != 0) {
            glActiveTexture(GL_TEXTURE0 + texture_idx);
            glBindTexture(
              shader_asset->texture_unit_types[texture_idx],
              shader_asset->texture_units[texture_idx]
            );
          }
        }
      }
    }

    for (
      uint32 uniform_idx = 0;
      uniform_idx < shader_asset->n_intrinsic_uniforms;
      uniform_idx++
    ) {
      const char *uniform_name = shader_asset->intrinsic_uniform_names[uniform_idx];
      if (str::eq(uniform_name, "model_matrix")) {
        shaders::set_mat4(shader_asset, "model_matrix", model_matrix);
      } else if (str::eq(uniform_name, "model_normal_matrix")) {
        shaders::set_mat3(shader_asset, "model_normal_matrix", model_normal_matrix);
      } else if (bone_matrices && str::eq(uniform_name, "bone_matrices[0]")) {
        shaders::set_mat4_multiple(
          shader_asset, MAX_N_BONES, "bone_matrices[0]", bone_matrices
        );
      }
    }

    Mesh *mesh = &drawable_component->mesh;
    glBindVertexArray(mesh->vao);
    if (mesh->n_indices > 0) {
      glDrawElements(mesh->mode, mesh->n_indices, GL_UNSIGNED_INT, 0);
    } else {
      glDrawArrays(mesh->mode, 0, mesh->n_vertices);
    }
  }


  void draw_all(
    EntitySet *entity_set,
    DrawableComponentSet *drawable_component_set,
    SpatialComponentSet *spatial_component_set,
    AnimationComponentSet *animation_component_set,
    Array<Material> *materials,
    RenderPassFlag render_pass,
    RenderMode render_mode,
    ShaderAsset *standard_depth_shader_asset,
    real64 t
  ) {
    ModelMatrixCache cache = {m4(1.0f), nullptr};

    for_each (drawable_component, drawable_component_set->components) {
      if (!models::is_drawable_component_valid(drawable_component)) {
        continue;
      }

      if (!(render_pass & drawable_component->target_render_pass)) {
        continue;
      }

#if 0
      logs::info(
        "Drawing %s",
        entity_set->entities[drawable_component->entity_handle]->debug_name
      );
#endif

      Material *material = materials::get_material_by_name(
        materials, drawable_component->mesh.material_name
      );

      if (!material || material->state != MaterialState::complete) {
        material = materials::get_material_by_name(materials, "unknown");
      }

      SpatialComponent *spatial_component =
        spatial_component_set->components[drawable_component->entity_handle];

      m4 model_matrix = m4(1.0f);
      m3 model_normal_matrix = m3(1.0f);
      m4 *bone_matrices = nullptr;

      if (spatial::is_spatial_component_valid(spatial_component)) {
        // We only need to calculate the normal matrix if we have non-uniform
        // scaling.
        model_matrix = make_model_matrix(spatial_component_set, spatial_component, &cache);

        // TODO: Even though we have a uniform scaling in the transformation for
        // our spatial component itself, when accumulating it with the parent
        // spatial components, we might (possibly) get non-uniform scaling,
        // in which case we want to calculate the model normal matrix!
        // Oops! We should be looking at the model_matrix and not at
        // spatial_component->scale.
        if (
          spatial_component->scale.x == spatial_component->scale.y &&
          spatial_component->scale.y == spatial_component->scale.z
        ) {
          model_normal_matrix = m3(model_matrix);
        } else {
          model_normal_matrix = m3(transpose(inverse(model_matrix)));
        }

        // Animations
        AnimationComponent *animation_component = find_animation_component(
          spatial_component,
          spatial_component_set,
          animation_component_set
        );
        if (animation_component) {
          bone_matrices = animation_component->bone_matrices;
        }
      }

      draw(
        render_mode,
        drawable_component_set,
        drawable_component,
        material,
        &model_matrix,
        &model_normal_matrix,
        bone_matrices,
        standard_depth_shader_asset
      );
    }
  }


  void render_scene(
    State *state,
    RenderPassFlag render_pass,
    RenderMode render_mode
  ) {
#if 0
    logs::info("RenderPass: %s", render_pass_to_string(render_pass));
#endif
    draw_all(
      &state->entity_set,
      &state->drawable_component_set,
      &state->spatial_component_set,
      &state->animation_component_set,
      &state->materials,
      render_pass,
      render_mode,
      &state->standard_depth_shader_asset,
      state->t
    );
  }


  void get_entity_text_representation(
    char *text,
    State *state,
    Entity *entity,
    uint8 depth
  ) {
    char number[128];

    EntityHandle handle = entity->handle;
    SpatialComponent *spatial_component =
      state->spatial_component_set.components[handle];

    // Children will be drawn under their parents.
    if (
      depth == 0 &&
      spatial::is_spatial_component_valid(spatial_component) &&
      spatial_component->parent_entity_handle != Entity::no_entity_handle
    ) {
      return;
    }

    bool32 has_spatial_component = spatial::is_spatial_component_valid(
      spatial_component
    );
    bool32 has_drawable_component = models::is_drawable_component_valid(
      state->drawable_component_set.components[handle]
    );
    bool32 has_light_component = lights::is_light_component_valid(
      state->light_component_set.components[handle]
    );
    bool32 has_behavior_component = behavior::is_behavior_component_valid(
      state->behavior_component_set.components[handle]
    );
    bool32 has_animation_component = anim::is_animation_component_valid(
      state->animation_component_set.components[handle]
    );

    for (uint8 level = 0; level < depth; level++) {
      strcat(text, "  ");
    }

    strcat(text, "- ");
    strcat(text, entity->debug_name);

    strcat(text, "@");
    _itoa(entity->handle, number, 10);
    strcat(text, number);

    if (
      !has_spatial_component &&
      !has_drawable_component &&
      !has_light_component &&
      !has_behavior_component &&
      !has_animation_component
    ) {
      strcat(text, " (orphan)");
    }

    if (has_spatial_component) {
      strcat(text, " +S");
    }
    if (has_drawable_component) {
      strcat(text, " +D");
    }
    if (has_light_component) {
      strcat(text, " +L");
    }
    if (has_behavior_component) {
      strcat(text, " +B");
    }
    if (has_animation_component) {
      strcat(text, " +A");
    }

    if (spatial::is_spatial_component_valid(spatial_component)) {
      // NOTE: This is super slow lol.
      uint32 n_children_found = 0;
      for_each (child_spatial_component, state->spatial_component_set.components) {
        if (
          child_spatial_component->parent_entity_handle ==
            spatial_component->entity_handle
        ) {
          n_children_found++;
          if (n_children_found > 5) {
            continue;
          }
          EntityHandle child_handle = child_spatial_component->entity_handle;
          Entity *child_entity = state->entity_set.entities[child_handle];

          if (text[strlen(text) - 1] != '\n') {
            strcat(text, "\n");
          }
          get_entity_text_representation(text, state, child_entity, depth + 1);
        }
      }
      if (n_children_found > 5) {
        for (uint8 level = 0; level < (depth + 1); level++) {
          strcat(text, "  ");
        }
        strcat(text, "(and ");
        _itoa(n_children_found - 5, number, 10);
        strcat(text, number);
        strcat(text, " more)");
      }
    }

    if (text[strlen(text) - 1] != '\n') {
      strcat(text, "\n");
    }
  }


  void get_scene_text_representation(char *text, State *state) {
    text[0] = '\0';

    for_each (entity, state->entity_set.entities) {
      get_entity_text_representation(text, state, entity, 0);
    }

    if (text[strlen(text) - 1] == '\n') {
      text[strlen(text) - 1] = '\0';
    }
  }


  void get_materials_text_representation(char *text, State *state) {
    text[0] = '\0';

    strcat(text, "Internal:\n");

    uint32 idx = 0;
    for_each (material, state->materials) {
      strcat(text, "- ");
      strcat(text, material->name);
      strcat(text, "\n");
      if (idx == state->first_non_internal_material_idx - 1) {
        strcat(text, "Non-internal: \n");
      }
      idx++;
    }

    if (text[strlen(text) - 1] == '\n') {
      text[strlen(text) - 1] = '\0';
    }
  }


  void render_scene_ui(State *state) {
    char debug_text[1 << 14];
    size_t dt_size = sizeof(debug_text);

    gui::start_drawing(&state->gui_state);

    if (state->gui_state.heading_opacity > 0.0f) {
      gui::draw_heading(
        &state->gui_state,
        state->gui_state.heading_text,
        v4(0.0f, 0.33f, 0.93f, state->gui_state.heading_opacity)
      );
      if (state->gui_state.heading_fadeout_delay > 0.0f) {
        state->gui_state.heading_fadeout_delay -= (real32)state->dt;
      } else {
        state->gui_state.heading_opacity -=
          state->gui_state.heading_fadeout_duration * (real32)state->dt;
      }
    }

    {
      strcpy(debug_text, "Peony debug info: ");
      strcat(debug_text, state->current_scene_name);
      GuiContainer *container = gui::make_container(
        &state->gui_state, debug_text, v2(25.0f, 25.0f)
      );

      snprintf(
        debug_text, dt_size, "%ux%u", state->window_info.width, state->window_info.height
      );
      gui::draw_named_value(&state->gui_state, container, "screen size", debug_text);

      snprintf(
        debug_text, dt_size, "%ux%u",
        state->window_info.screencoord_width, state->window_info.screencoord_height
      );
      gui::draw_named_value(&state->gui_state, container, "window size", debug_text);

      snprintf(debug_text, dt_size, "%u fps", state->perf_counters.last_fps);
      gui::draw_named_value(&state->gui_state, container, "fps", debug_text);

      snprintf(debug_text, dt_size, "%.2f ms", state->perf_counters.dt_average * 1000.0f);
      gui::draw_named_value(&state->gui_state, container, "dt", debug_text);

      snprintf(debug_text, dt_size, "%.2f", 1.0f + state->timescale_diff);
      gui::draw_named_value(&state->gui_state, container, "ts", debug_text);

      snprintf(debug_text, dt_size, state->is_world_loaded ? "yes" : "no");
      gui::draw_named_value(&state->gui_state, container, "is_world_loaded", debug_text);

      snprintf(debug_text, dt_size, "%u", state->materials.length);
      gui::draw_named_value(
        &state->gui_state, container, "materials.length", debug_text
      );

      snprintf(debug_text, dt_size, "%u", state->entity_set.entities.length);
      gui::draw_named_value(&state->gui_state, container, "entities.length", debug_text);

      snprintf(debug_text, dt_size, "%u", state->model_loaders.length);
      gui::draw_named_value(
        &state->gui_state, container, "model_loaders.length", debug_text
      );

      snprintf(debug_text, dt_size, "%u", state->n_valid_model_loaders);
      gui::draw_named_value(
        &state->gui_state, container, "n_valid_model_loaders", debug_text
      );

      snprintf(debug_text, dt_size, "%u", state->entity_loader_set.loaders.length);
      gui::draw_named_value(
        &state->gui_state, container, "entity_loader_set.length", debug_text
      );

      snprintf(debug_text, dt_size, "%u", state->n_valid_entity_loaders);
      gui::draw_named_value(
        &state->gui_state, container, "n_valid_entity_loaders", debug_text
      );

      if (gui::draw_toggle(
        &state->gui_state, container, "Wireframe mode", &state->should_use_wireframe
      )) {
        state->should_use_wireframe = !state->should_use_wireframe;
        if (state->should_use_wireframe) {
          gui::set_heading(&state->gui_state, "Wireframe mode on.", 1.0f, 1.0f, 1.0f);
        } else {
          gui::set_heading(&state->gui_state, "Wireframe mode off.", 1.0f, 1.0f, 1.0f);
        }
      }

      if (gui::draw_toggle(
        &state->gui_state, container, "FPS limit", &state->should_limit_fps
      )) {
        state->should_limit_fps = !state->should_limit_fps;
        if (state->should_limit_fps) {
          gui::set_heading(&state->gui_state, "FPS limit enabled.", 1.0f, 1.0f, 1.0f);
        } else {
          gui::set_heading(&state->gui_state, "FPS limit disabled.", 1.0f, 1.0f, 1.0f);
        }
      }

      if (gui::draw_toggle(
        &state->gui_state, container, "Manual frame advance", &state->is_manual_frame_advance_enabled
      )) {
        state->is_manual_frame_advance_enabled = !state->is_manual_frame_advance_enabled;
        if (state->is_manual_frame_advance_enabled) {
          gui::set_heading(
            &state->gui_state, "Manual frame advance enabled.", 1.0f, 1.0f, 1.0f
          );
        } else {
          gui::set_heading(
            &state->gui_state, "Manual frame advance disabled.", 1.0f, 1.0f, 1.0f
          );
        }
      }

      if (gui::draw_toggle(
        &state->gui_state, container, "Pause", &state->should_pause
      )) {
        state->should_pause = !state->should_pause;
        if (state->should_pause) {
          gui::set_heading(&state->gui_state, "Pause enabled.", 1.0f, 1.0f, 1.0f);
        } else {
          gui::set_heading(&state->gui_state, "Pause disabled.", 1.0f, 1.0f, 1.0f);
        }
      }

      if (gui::draw_button(
        &state->gui_state, container, "Reload shaders"
      )) {
        reload_shaders(state);
        gui::set_heading(&state->gui_state, "Shaders reloaded.", 1.0f, 1.0f, 1.0f);
      }

      if (gui::draw_button(
        &state->gui_state, container, "Delete PBO"
      )) {
        materials::delete_persistent_pbo(&state->persistent_pbo);
        gui::set_heading(&state->gui_state, "PBO deleted.", 1.0f, 1.0f, 1.0f);
      }
    }

    {
#if 1
      GuiContainer *container = gui::make_container(
        &state->gui_state, "Entities", v2(state->window_info.width - 400.0f, 25.0f)
      );
      get_scene_text_representation(debug_text, state);
      gui::draw_body_text(&state->gui_state, container, debug_text);
#endif
    }

    {
#if 1
      GuiContainer *container = gui::make_container(
        &state->gui_state, "Materials", v2(state->window_info.width - 600.0f, 25.0f)
      );
      get_materials_text_representation(debug_text, state);
      gui::draw_body_text(&state->gui_state, container, debug_text);
#endif
    }

    gui::draw_console(
      &state->gui_state, state->input_state.text_input
    );
    gui::render(&state->gui_state);
  }


  void render(State *state) {
    copy_scene_data_to_ubo(state);

    // Clear framebuffers
    {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glBindFramebuffer(GL_FRAMEBUFFER, state->builtin_textures.g_buffer);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glBindFramebuffer(GL_FRAMEBUFFER, state->builtin_textures.l_buffer);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glBindFramebuffer(GL_FRAMEBUFFER, state->builtin_textures.blur1_buffer);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      glBindFramebuffer(GL_FRAMEBUFFER, state->builtin_textures.blur2_buffer);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    // Render shadow map
    {
      // Point lights
      {
        m4 perspective_projection = glm::perspective(
          radians(90.0f),
          (
            (real32)state->builtin_textures.shadowmap_3d_width /
            (real32)state->builtin_textures.shadowmap_3d_height
          ),
          state->builtin_textures.shadowmap_near_clip_dist,
          state->builtin_textures.shadowmap_far_clip_dist
        );

        uint32 idx_light = 0;

        for_each (light_component, state->light_component_set.components) {
          if (light_component->entity_handle == Entity::no_entity_handle) {
            continue;
          }

          SpatialComponent *spatial_component =
            state->spatial_component_set.components[light_component->entity_handle];

          if (!(
            lights::is_light_component_valid(light_component) &&
            light_component->type == LightType::point &&
            spatial::is_spatial_component_valid(spatial_component)
          )) {
            continue;
          }

          v3 position = spatial_component->position;

          for (uint32 idx_face = 0; idx_face < 6; idx_face++) {
            state->shadowmap_3d_transforms[(idx_light * 6) + idx_face] =
              perspective_projection * glm::lookAt(
                position,
                position + models::CUBEMAP_OFFSETS[idx_face],
                models::CUBEMAP_UPS[idx_face]
              );
          }

          glViewport(
            0, 0,
            state->builtin_textures.shadowmap_3d_width,
            state->builtin_textures.shadowmap_3d_height
          );
          glBindFramebuffer(
            GL_FRAMEBUFFER,
            state->builtin_textures.shadowmaps_3d_framebuffer
          );
          glClear(GL_DEPTH_BUFFER_BIT);

          copy_scene_data_to_ubo(
            state, idx_light, lights::light_type_to_int(light_component->type), false
          );
          render_scene(
            state,
            RenderPass::shadowcaster,
            RenderMode::depth
          );

          idx_light++;
        }
      }

      // Directional lights
      {
        real32 ortho_ratio = (
          (real32)state->builtin_textures.shadowmap_2d_width /
          (real32)state->builtin_textures.shadowmap_2d_height
        );
        real32 ortho_width = 100.0f;
        real32 ortho_height = ortho_width / ortho_ratio;
        m4 ortho_projection = glm::ortho(
          -ortho_width, ortho_width,
          -ortho_height, ortho_height,
          state->builtin_textures.shadowmap_near_clip_dist,
          state->builtin_textures.shadowmap_far_clip_dist
        );

        uint32 idx_light = 0;

        for_each (light_component, state->light_component_set.components) {
          if (light_component->entity_handle == Entity::no_entity_handle) {
            continue;
          }

          SpatialComponent *spatial_component =
            state->spatial_component_set.components[light_component->entity_handle];

          if (!(
            lights::is_light_component_valid(light_component) &&
            light_component->type == LightType::directional &&
            spatial::is_spatial_component_valid(spatial_component)
          )) {
            continue;
          }

          state->shadowmap_2d_transforms[idx_light] = ortho_projection * glm::lookAt(
            spatial_component->position,
            spatial_component->position + light_component->direction,
            v3(0.0f, -1.0f, 0.0f)
          );

          glViewport(
            0, 0,
            state->builtin_textures.shadowmap_2d_width,
            state->builtin_textures.shadowmap_2d_height
          );
          glBindFramebuffer(
            GL_FRAMEBUFFER,
            state->builtin_textures.shadowmaps_2d_framebuffer
          );
          glClear(GL_DEPTH_BUFFER_BIT);

          copy_scene_data_to_ubo(
            state, idx_light, lights::light_type_to_int(light_component->type), false
          );
          render_scene(
            state,
            RenderPass::shadowcaster,
            RenderMode::depth
          );

          idx_light++;
        }
      }
    }

    glViewport(
      0, 0, state->window_info.width, state->window_info.height
    );

    // Geometry pass
    {
      if (state->should_use_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      }
      glBindFramebuffer(GL_FRAMEBUFFER, state->builtin_textures.g_buffer);
      render_scene(
        state,
        RenderPass::deferred,
        RenderMode::regular
      );
      if (state->should_use_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      }
    }

    // Copy depth from geometry pass to lighting pass
    {
      glBindFramebuffer(GL_READ_FRAMEBUFFER, state->builtin_textures.g_buffer);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, state->builtin_textures.l_buffer);
      glBlitFramebuffer(
        0, 0, state->window_info.width, state->window_info.height,
        0, 0, state->window_info.width, state->window_info.height,
        GL_DEPTH_BUFFER_BIT, GL_NEAREST
      );
    }

    glBindFramebuffer(GL_FRAMEBUFFER, state->builtin_textures.l_buffer);

    // Lighting pass
    {
      glDisable(GL_DEPTH_TEST);
      render_scene(
        state,
        RenderPass::lighting,
        RenderMode::regular
      );
      glEnable(GL_DEPTH_TEST);
    }


    // Forward pass
    {
      // Skysphere
      {
        // Cull outside, not inside, of sphere.
        glCullFace(GL_FRONT);
        // Do not write to depth buffer.
        glDepthMask(GL_FALSE);
        // Draw at the very back of our depth range, so as to be behind everything.
        glDepthRange(0.9999f, 1.0f);

        render_scene(
          state,
          RenderPass::forward_skybox,
          RenderMode::regular
        );

        glDepthRange(0.0f, 1.0f);
        glDepthMask(GL_TRUE);
        glCullFace(GL_BACK);
      }

      // Forward
      {
        if (state->should_use_wireframe) {
          glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        render_scene(
          state,
          RenderPass::forward_depth,
          RenderMode::regular
        );

        glDisable(GL_DEPTH_TEST);
        render_scene(
          state,
          RenderPass::forward_nodepth,
          RenderMode::regular
        );
        glEnable(GL_DEPTH_TEST);

        if (state->should_use_wireframe) {
          glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
      }

      // Debug draw pass
      {
        debugdraw::render(&state->debug_draw_state);
      }
    }

    glDisable(GL_DEPTH_TEST);

#if USE_BLOOM
    // Blur pass
    {
      glBindFramebuffer(GL_FRAMEBUFFER, state->blur1_buffer);
      copy_scene_data_to_ubo(state, 0, 0, true);
      render_scene(
        state, RenderPass::preblur, RenderMode::regular
      );

      glBindFramebuffer(GL_FRAMEBUFFER, state->blur2_buffer);
      copy_scene_data_to_ubo(state, 0, 0, false);
      render_scene(
        state, RenderPass::blur2, RenderMode::regular
      );

      for (uint32 idx = 0; idx < 3; idx++) {
        glBindFramebuffer(GL_FRAMEBUFFER, state->blur1_buffer);
        copy_scene_data_to_ubo(state, 0, 0, true);
        render_scene(
          state, RenderPass::blur1, RenderMode::regular
        );

        glBindFramebuffer(GL_FRAMEBUFFER, state->blur2_buffer);
        copy_scene_data_to_ubo(state, 0, 0, false);
        render_scene(
          state, RenderPass::blur1, RenderMode::regular
        );
      }
    }
#endif

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Postprocessing pass
    {
      render_scene(
        state,
        RenderPass::postprocessing,
        RenderMode::regular
      );
    }

    // Debug pass
    {
      render_scene(
        state,
        RenderPass::renderdebug,
        RenderMode::regular
      );
    }

    // UI pass
    {
      glEnable(GL_BLEND);
      if (!state->should_hide_ui) {
        render_scene_ui(state);
      }
      glDisable(GL_BLEND);
    }

    glEnable(GL_DEPTH_TEST);
  }


  // -----------------------------------------------------------
  // Public functions
  // -----------------------------------------------------------
}
