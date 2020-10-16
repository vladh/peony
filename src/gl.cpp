#define _CRT_SECURE_NO_WARNINGS

#define USE_OPENGL_4 false
#define USE_OPENGL_DEBUG false
#define USE_TIMERS true
#define USE_NO_WINDOW_DECORATION false

#include "gl.hpp"

global_variable uint32 global_oopses = 0;

#include "log.cpp"
#include "pack.cpp"
#include "util.cpp"
#include "task.cpp"
#include "resource_manager.cpp"
#include "texture_name_pool.cpp"
#include "font_asset.cpp"
#include "shader_asset.cpp"
#include "persistent_pbo.cpp"
#include "texture_set.cpp"
#include "camera.cpp"
#include "memory.cpp"
#include "control.cpp"
#include "entity.cpp"
#include "entity_manager.cpp"
#include "drawable_component_manager.cpp"
#include "light_component_manager.cpp"
#include "spatial_component_manager.cpp"
#include "text_manager.cpp"
#include "model_asset.cpp"
#include "scene.cpp"
#include "scene_resources.cpp"
#include "state.cpp"


void init_ubo(Memory *memory, State *state) {
  glGenBuffers(1, &state->ubo_shader_common);
  glBindBuffer(GL_UNIFORM_BUFFER, state->ubo_shader_common);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderCommon), NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  glBindBufferRange(GL_UNIFORM_BUFFER, 0, state->ubo_shader_common, 0, sizeof(ShaderCommon));
}


void init_shadow_buffers(Memory *memory, State *state) {
  state->n_shadow_framebuffers = state->lights.size;

  for (
    uint32 idx_framebuffer = 0;
    idx_framebuffer < MAX_N_SHADOW_FRAMEBUFFERS;
    idx_framebuffer++
  ) {
    glGenFramebuffers(1, &state->shadow_framebuffers[idx_framebuffer]);
    glGenTextures(1, &state->shadow_cubemaps[idx_framebuffer]);
    glBindTexture(GL_TEXTURE_CUBE_MAP, state->shadow_cubemaps[idx_framebuffer]);

    for (uint32 idx_face = 0; idx_face < 6; idx_face++) {
      glTexImage2D(
        GL_TEXTURE_CUBE_MAP_POSITIVE_X + idx_face, 0, GL_DEPTH_COMPONENT,
        state->shadow_map_width, state->shadow_map_height,
        0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL
      );
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, state->shadow_framebuffers[idx_framebuffer]);
    glFramebufferTexture(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, state->shadow_cubemaps[idx_framebuffer], 0
    );
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
}


void init_g_buffer(Memory *memory, State *state) {
  glGenFramebuffers(1, &state->g_buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, state->g_buffer);

  uint32 g_position_texture_name;
  uint32 g_normal_texture_name;
  uint32 g_albedo_texture_name;
  uint32 g_pbr_texture_name;

  glGenTextures(1, &g_position_texture_name);
  glGenTextures(1, &g_normal_texture_name);
  glGenTextures(1, &g_albedo_texture_name);
  glGenTextures(1, &g_pbr_texture_name);

  state->g_position_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "g_position_texture")
  ) Texture(
    GL_TEXTURE_2D,
    TEXTURE_G_POSITION, "g_position_texture", g_position_texture_name,
    state->window_info.width, state->window_info.height, 4
  );
  state->g_normal_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "g_normal_texture")
  ) Texture(
    GL_TEXTURE_2D,
    TEXTURE_G_NORMAL, "g_normal_texture", g_normal_texture_name,
    state->window_info.width, state->window_info.height, 4
  );
  state->g_albedo_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "g_albedo_texture")
  ) Texture(
    GL_TEXTURE_2D,
    TEXTURE_G_ALBEDO, "g_albedo_texture", g_albedo_texture_name,
    state->window_info.width, state->window_info.height, 4
  );
  state->g_pbr_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "g_pbr_texture")
  ) Texture(
    GL_TEXTURE_2D,
    TEXTURE_G_PBR, "g_pbr_texture", g_pbr_texture_name,
    state->window_info.width, state->window_info.height, 4
  );

  glBindTexture(GL_TEXTURE_2D, state->g_position_texture->texture_name);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA16F,
    state->g_position_texture->width, state->g_position_texture->height,
    0, GL_RGBA, GL_FLOAT, NULL
  );
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
    state->g_position_texture->texture_name, 0
  );

  glBindTexture(GL_TEXTURE_2D, state->g_normal_texture->texture_name);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA16F,
    state->g_normal_texture->width, state->g_normal_texture->height,
    0, GL_RGBA, GL_FLOAT, NULL
  );
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
    state->g_normal_texture->texture_name, 0
  );

  glBindTexture(GL_TEXTURE_2D, state->g_albedo_texture->texture_name);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA,
    state->g_albedo_texture->width, state->g_albedo_texture->height,
    0, GL_RGBA, GL_UNSIGNED_BYTE, NULL
  );
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
    state->g_albedo_texture->texture_name, 0
  );

  glBindTexture(GL_TEXTURE_2D, state->g_pbr_texture->texture_name);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA,
    state->g_pbr_texture->width, state->g_pbr_texture->height,
    0, GL_RGBA, GL_UNSIGNED_BYTE, NULL
  );
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D,
    state->g_pbr_texture->texture_name, 0
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
    state->window_info.width, state->window_info.height
  );
  glFramebufferRenderbuffer(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth
  );

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    log_error("Framebuffer not complete!");
  }
}


void init_screenquad(Memory *memory, State *state) {
  real32 screenquad_vertices[] = SCREENQUAD_VERTICES;
  ShaderAsset *shader_asset = new(state->shader_assets.push()) ShaderAsset(
    memory,
    "lighting",
    SHADER_LIGHTING,
    SHADER_DIR"lighting.vert", SHADER_DIR"lighting.frag"
  );

  ModelAsset *model_asset = new(state->model_assets.push()) ModelAsset(
    memory,
    MODELSOURCE_DATA,
    screenquad_vertices, 6,
    nullptr, 0,
    "screenquad",
    GL_TRIANGLES
  );
  TextureSet *texture_set = new(model_asset->texture_sets.push()) TextureSet(memory);
  texture_set->add(*state->g_position_texture);
  texture_set->add(*state->g_normal_texture);
  texture_set->add(*state->g_albedo_texture);
  texture_set->add(*state->g_pbr_texture);
  for (uint32 idx = 0; idx < MAX_N_SHADOW_FRAMEBUFFERS; idx++) {
    texture_set->add(Texture(
        GL_TEXTURE_CUBE_MAP,
        TEXTURE_DEPTH, DEPTH_TEXTURE_UNIFORM_NAMES[idx], state->shadow_cubemaps[idx],
        state->shadow_map_width, state->shadow_map_height, 1
    ));
  }
  *model_asset->mesh_templates.push() = {
    shader_asset, nullptr, texture_set, true, 0, 0
  };

  Entity *entity = state->entity_manager.add("screenquad");
  state->drawable_component_manager.add(
    entity->handle,
    ModelAsset::get_by_name(&state->model_assets, "screenquad"),
    RENDERPASS_LIGHTING
  );
}


void update_drawing_options(State *state, GLFWwindow *window) {
  if (state->is_cursor_disabled) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  } else {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }

  if (state->should_use_wireframe) {
    // This will be handled in the rendering loop.
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
}


void process_input_continuous(GLFWwindow *window, State *state) {
  if (state->control.is_key_down(GLFW_KEY_W)) {
    state->camera_active->move_front_back(1);
  }

  if (state->control.is_key_down(GLFW_KEY_S)) {
    state->camera_active->move_front_back(-1);
  }

  if (state->control.is_key_down(GLFW_KEY_A)) {
    state->camera_active->move_left_right(-1);
  }

  if (state->control.is_key_down(GLFW_KEY_D)) {
    state->camera_active->move_left_right(1);
  }

  if (state->control.is_key_down(GLFW_KEY_SPACE)) {
    state->camera_active->move_up_down(1);
  }

  if (state->control.is_key_down(GLFW_KEY_LEFT_CONTROL)) {
    state->camera_active->move_up_down(-1);
  }
}


void process_input_transient(GLFWwindow *window, State *state) {
  if (state->control.is_key_now_down(GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(window, true);
  }

  if (state->control.is_key_now_down(GLFW_KEY_F)) {
    state->should_limit_fps = !state->should_limit_fps;
    update_drawing_options(state, window);
  }

  if (state->control.is_key_now_down(GLFW_KEY_C)) {
    state->is_cursor_disabled = !state->is_cursor_disabled;
    update_drawing_options(state, window);
  }

  if (state->control.is_key_now_down(GLFW_KEY_Q)) {
    state->should_use_wireframe = !state->should_use_wireframe;
    update_drawing_options(state, window);
  }

  if (state->control.is_key_now_down(GLFW_KEY_TAB)) {
    state->should_pause = !state->should_pause;
  }

  if (state->control.is_key_now_down(GLFW_KEY_BACKSPACE)) {
    state->should_hide_ui = !state->should_hide_ui;
  }

  if (state->control.is_key_now_down(GLFW_KEY_GRAVE_ACCENT)) {
    log_info("Deleting PBO");
    state->persistent_pbo.delete_pbo();
  }

  if (state->control.is_key_down(GLFW_KEY_P)) {
    state->is_manual_frame_advance_enabled = !state->is_manual_frame_advance_enabled;
  }

  if (state->control.is_key_down(GLFW_KEY_ENTER)) {
    state->should_manually_advance_to_next_frame = true;
  }
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
  State *state = memory_and_state->state;
  Memory *memory = memory_and_state->memory;
  log_info(
    "Window is now: %d x %d", state->window_info.width, state->window_info.height
  );
  state->window_info.width = width;
  state->window_info.height = height;
  state->text_manager.update_text_projection(width, height);

  // TODO: Only regenerate all this shit once we're done resizing, not for every little bit of the resize.
  init_g_buffer(memory, state);

  // TODO: I hate this.
  for (
    uint32 idx_component = 0;
    idx_component < state->drawable_component_manager.components->size;
    idx_component++
  ) {
    DrawableComponent *component = state->drawable_component_manager.components->get(
      idx_component
    );
    ModelAsset *model_asset = component->model_asset;
    for (
      uint32 idx_mesh = 0; idx_mesh < model_asset->meshes.size; idx_mesh++
    ) {
      Mesh *mesh = model_asset->meshes.get(idx_mesh);
      if (mesh->texture_set && mesh->texture_set->is_g_buffer_dependent) {
        log_info("Found G-buffer dependent mesh in model %s", model_asset->name);
        for(
          uint32 idx_texture = 0; idx_texture < mesh->texture_set->textures.size; idx_texture++
        ) {
          Texture *texture = mesh->texture_set->textures.get(idx_texture);
          if (texture->type == TEXTURE_G_POSITION) {
            mesh->texture_set->textures.set(idx_texture, state->g_position_texture);
          } else if (texture->type == TEXTURE_G_NORMAL) {
            mesh->texture_set->textures.set(idx_texture, state->g_normal_texture);
          } else if (texture->type == TEXTURE_G_ALBEDO) {
            mesh->texture_set->textures.set(idx_texture, state->g_albedo_texture);
          } else if (texture->type == TEXTURE_G_PBR) {
            mesh->texture_set->textures.set(idx_texture, state->g_pbr_texture);
          }
        }
        model_asset->bind_texture_uniforms_for_mesh(mesh);
      }
    }
  }
}


void mouse_callback(GLFWwindow *window, real64 x, real64 y) {
  MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
  State *state = memory_and_state->state;
  glm::vec2 mouse_offset = state->control.update_mouse(x, y);
  state->camera_active->update_mouse(mouse_offset);
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
  State *state = memory_and_state->state;
  state->control.update_keys(key, scancode, action, mods);
  process_input_transient(window, state);
}


void init_window(WindowInfo *window_info) {
  strcpy(window_info->title, "hi lol");

  glfwInit();

#if USE_OPENGL_4
  log_info("Using OpenGL 4.3");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#else
  log_info("Using OpenGL 3.3");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
#endif

  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

#if defined(__APPLE__)
  log_info("Using GLFW_OPENGL_FORWARD_COMPAT");
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

#if USE_OPENGL_DEBUG
  log_info("Using OpenGL debug context");
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

#if USE_NO_WINDOW_DECORATION
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
#endif

  int32 n_monitors;
  GLFWmonitor **monitors = glfwGetMonitors(&n_monitors);
  GLFWmonitor *target_monitor = monitors[0];

  const GLFWvidmode *video_mode = glfwGetVideoMode(target_monitor);
  glfwWindowHint(GLFW_RED_BITS, video_mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, video_mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, video_mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, video_mode->refreshRate);
  window_info->width = video_mode->width;
  window_info->height = video_mode->height;
  /* window_info->width = 800; */
  /* window_info->height = 600; */

  GLFWwindow *window = glfwCreateWindow(
    window_info->width, window_info->height, window_info->title,
    /* target_monitor, nullptr */
    nullptr, nullptr
  );
  if (!window) {
    log_fatal("Failed to create GLFW window");
    return;
  }
  window_info->window = window;
  glfwSetWindowPos(window, 0, 0);
  /* glfwSetWindowPos(window, 200, 200); */

  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    log_fatal("Failed to initialize GLAD");
    return;
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);

#if USE_OPENGL_DEBUG
  GLint flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

  if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(Util::debug_message_callback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
  } else {
    log_fatal("Tried to initialise OpenGL debug output but couldn't");
  }
#endif

  glEnable(GL_CULL_FACE);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport(0, 0, window_info->width, window_info->height);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetKeyCallback(window, key_callback);
}


void copy_scene_data_to_ubo(Memory *memory, State *state) {
  ShaderCommon *shader_common = &state->shader_common;

  shader_common->far_clip_dist = state->shadow_far_clip_dist;
  shader_common->shadow_light_idx = state->shadow_light_idx;

  memcpy(shader_common->shadow_transforms, state->shadow_transforms, sizeof(state->shadow_transforms));
  shader_common->view = state->camera_active->view;
  shader_common->projection = state->camera_active->projection;
  shader_common->camera_position = glm::vec4(state->camera_active->position, 1.0f);
  shader_common->exposure = state->camera_active->exposure;
  shader_common->t = (float)state->t;

  shader_common->n_lights = state->lights.size;
  for (uint32 idx = 0; idx < state->lights.size; idx++) {
    SpatialComponent *spatial_component =
      state->spatial_component_manager.get(*state->lights.get(idx));
    LightComponent *light_component =
      state->light_component_manager.get(*state->lights.get(idx));
    shader_common->light_position[idx] = glm::vec4(spatial_component->position, 1.0f);
    shader_common->light_color[idx] = light_component->color;
    shader_common->light_attenuation[idx] = light_component->attenuation;
  }

  glBindBuffer(GL_UNIFORM_BUFFER, state->ubo_shader_common);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderCommon), shader_common);
}


void render_scene(
  Memory *memory, State *state, RenderPass render_pass, RenderMode render_mode
) {
  state->drawable_component_manager.draw_all(
    memory,
    &state->persistent_pbo,
    &state->texture_name_pool,
    &state->spatial_component_manager,
    &state->task_queue,
    render_pass, render_mode, state->standard_depth_shader_asset
  );
}


void render_scene_ui(
  Memory *memory, State *state
){
  char debug_text[256];
  sprintf(
    debug_text,
    "(fps %.2f)\n"
    "(t %f)\n"
    "(dt %f)\n"
    "(frame %d)\n"
    "(should_limit_fps %d)\n"
    "(oopses %d)",
    state->last_fps,
    state->t,
    state->dt,
    state->n_frames_since_start,
    state->should_limit_fps,
    global_oopses
  );
  state->text_manager.draw(
    "main-font", debug_text,
    15.0f, state->window_info.height - 35.0f,
    1.0f, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
  );
}


void scene_update(Memory *memory, State *state) {
  // TODO: Eventually move this to some kind of ActorComponent system.
  // We should rather be iterating through all SpatialComponents rather
  // than looking everything up.

  // Lights
  {
    EntityHandle *handle = state->lights.get(0);

    if (handle) {
      SpatialComponent *spatial = state->spatial_component_manager.get(*handle);

      if (spatial) {
        real64 time_term =
          (sin(state->t / 1.5f) + 1.0f) / 2.0f * (PI / 2.0f) + (PI / 2.0f);
        real64 x_term = 0.0f + cos(time_term) * 8.0f;
        real64 z_term = 0.0f + sin(time_term) * 8.0f;

        spatial->position.x = (real32)x_term;
        spatial->position.z = (real32)z_term;
      }
    }
  }

  // Geese
#if 0
  {
    real32 spin_deg_per_t = 90.0f;
    for (uint32 idx = 0; idx < state->geese.size; idx++) {
      SpatialComponent *spatial = state->spatial_component_manager.get(*state->geese.get(idx));
      spatial->rotation *= glm::angleAxis(
        glm::radians(spin_deg_per_t * (real32)state->dt),
        glm::vec3(0.0f, 1.0f, 0.0f)
      );
    }
  }
#endif

  // Spheres
#if 0
  {
    real32 spin_deg_per_t = 45.0f;
    for (uint32 idx = 0; idx < state->spheres.size; idx++) {
      SpatialComponent *spatial = state->spatial_component_manager.get(*state->spheres.get(idx));
      spatial->rotation *= glm::angleAxis(
        glm::radians(spin_deg_per_t * (real32)state->dt),
        glm::vec3(0.0f, 1.0f, 0.0f)
      );
    }
  }
#endif
}


void update_and_render(Memory *memory, State *state) {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  scene_update(memory, state);
  state->camera_active->update_matrices(
    state->window_info.width, state->window_info.height
  );
  copy_scene_data_to_ubo(memory, state);

  // Clear main framebuffer
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  // Render shadow map
  {
    glViewport(
      0, 0, state->shadow_map_width, state->shadow_map_height
    );
    for (uint32 idx = 0; idx < state->n_shadow_framebuffers; idx++) {
      SpatialComponent *spatial_component =
        state->spatial_component_manager.get(*state->lights.get(idx));
      Camera::create_shadow_transforms(
        state->shadow_transforms, spatial_component->position,
        state->shadow_map_width, state->shadow_map_height,
        state->shadow_near_clip_dist, state->shadow_far_clip_dist
      );

      glBindFramebuffer(GL_FRAMEBUFFER, state->shadow_framebuffers[idx]);
      glClear(GL_DEPTH_BUFFER_BIT);

      state->shadow_light_idx = idx;
      copy_scene_data_to_ubo(memory, state);
      render_scene(
        memory, state, RENDERPASS_DEFERRED, RENDERMODE_DEPTH
      );
    }
    glViewport(
      0, 0, state->window_info.width, state->window_info.height
    );
  }

  // Geometry pass
  {
    if (state->should_use_wireframe) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, state->g_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render_scene(memory, state, RENDERPASS_DEFERRED, RENDERMODE_REGULAR);
    if (state->should_use_wireframe) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  }

  // Copy depth from geometry pass to lighting pass
  {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, state->g_buffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(
      0, 0, state->window_info.width, state->window_info.height,
      0, 0, state->window_info.width, state->window_info.height,
      GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
  }

  // Lighting pass
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClearColor(
      state->background_color.r, state->background_color.g,
      state->background_color.b, state->background_color.a
    );
    glClear(GL_COLOR_BUFFER_BIT);
    render_scene(memory, state, RENDERPASS_LIGHTING, RENDERMODE_REGULAR);
    glEnable(GL_DEPTH_TEST);
  }


  glEnable(GL_BLEND);
  // Forward pass
  {
    if (state->should_use_wireframe) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    render_scene(memory, state, RENDERPASS_FORWARD, RENDERMODE_REGULAR);
    if (state->should_use_wireframe) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  }

  // UI pass
  {
    if (!state->should_hide_ui) {
      render_scene_ui(memory, state);
    }
  }
  glDisable(GL_BLEND);
}


void run_main_loop(Memory *memory, State *state) {
  std::chrono::steady_clock::time_point second_start = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point frame_start = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point last_frame_start = std::chrono::steady_clock::now();
  // 1/165 seconds (for 165 fps)
  std::chrono::nanoseconds frame_duration = std::chrono::nanoseconds(6060606);
  std::chrono::steady_clock::time_point time_frame_should_end;
  uint32 n_frames_this_second = 0;
  state->n_frames_since_start = 0;

  while (!state->should_stop) {
    glfwPollEvents();
    process_input_continuous(state->window_info.window, state);

    if (
      !state->is_manual_frame_advance_enabled ||
      state->should_manually_advance_to_next_frame
    ) {
      state->n_frames_since_start++;
      last_frame_start = frame_start;
      frame_start = std::chrono::steady_clock::now();
      time_frame_should_end = frame_start + frame_duration;

      // If we should pause, stop time-based events.
      if (!state->should_pause) {
        state->dt = std::chrono::duration_cast<std::chrono::duration<float>>(
          frame_start - last_frame_start
        ).count();
        state->t += state->dt;
      }

      // Count FPS.
      n_frames_this_second++;
      std::chrono::duration<real64> time_since_second_start = frame_start - second_start;
      if (time_since_second_start >= std::chrono::seconds(1)) {
        second_start = frame_start;
        state->last_fps = n_frames_this_second;
        n_frames_this_second = 0;
      }

      update_and_render(memory, state);
      if (state->is_manual_frame_advance_enabled) {
        state->should_manually_advance_to_next_frame = false;
      }

      if (state->should_limit_fps) {
        std::this_thread::sleep_until(time_frame_should_end);
      }
    }

    START_TIMER(swap_buffers);
    glfwSwapBuffers(state->window_info.window);
    END_TIMER_MIN(swap_buffers, 5);

    if (glfwWindowShouldClose(state->window_info.window)) {
      state->should_stop = true;
    }
  }
}


void destroy_window() {
  glfwTerminate();
}


void run_loading_loop(std::mutex *mutex, Memory *memory, State *state) {
  while (!state->should_stop) {
    Task *task = nullptr;

    mutex->lock();
    if (state->task_queue.size > 0) {
      task = state->task_queue.pop();
    }
    mutex->unlock();

    if (task) {
      task->run();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}


int main() {
  START_TIMER(init);

  srand((uint32)time(NULL));
  Memory memory;

  WindowInfo window_info;
  init_window(&window_info);
  if (!window_info.window) {
    return -1;
  }

  State *state = new((State*)memory.state_memory) State(&memory, window_info);

  std::mutex loading_thread_mutex;
  std::thread loading_thread_1 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state);
  std::thread loading_thread_2 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state);
  std::thread loading_thread_3 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state);

#if 0
  Util::print_texture_internalformat_info(GL_RGB8);
  Util::print_texture_internalformat_info(GL_RGBA8);
  Util::print_texture_internalformat_info(GL_SRGB8);
#endif

  update_drawing_options(state, window_info.window);

  MemoryAndState memory_and_state = {&memory, state};
  glfwSetWindowUserPointer(window_info.window, &memory_and_state);

  state->texture_name_pool.allocate_texture_names();

  init_g_buffer(&memory, state);
  init_ubo(&memory, state);

  scene_resources_init(&memory, state);
  scene_init_objects(&memory, state);

  init_shadow_buffers(&memory, state);
  init_screenquad(&memory, state);

  // NOTE: For some reason, this has to happen after `init_shadow_buffers()`
  // and before `allocate_texture_names()` or we get a lot of lag at the
  // beginning...?
  state->persistent_pbo.allocate_pbo();

#if 0
  memory.asset_memory_pool.print();
  memory.entity_memory_pool.print();
  memory.temp_memory_pool.print();
#endif
  log_info("Cache line size: %dB", cacheline_get_size());

  END_TIMER(init);

  run_main_loop(&memory, state);

  loading_thread_1.join();
  loading_thread_2.join();
  loading_thread_3.join();

  destroy_window();
  return 0;
}
