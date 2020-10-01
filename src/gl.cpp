#define _CRT_SECURE_NO_WARNINGS

#define USE_OPENGL_4 false
#define USE_OPENGL_DEBUG false
#define USE_1440P true

#include "gl.hpp"

global_variable uint32 global_oopses = 0;

#include "log.cpp"
#include "pack.cpp"
#include "util.cpp"
#include "resource_manager.cpp"
#include "font.cpp"
#include "shader.cpp"
#include "camera.cpp"
#include "memory.cpp"
#include "control.cpp"
#include "entity.cpp"
#include "entity_manager.cpp"
#include "drawable_component_manager.cpp"
#include "light_component_manager.cpp"
#include "spatial_component_manager.cpp"
#include "models.cpp"
#include "scene.cpp"
#include "scene_resources.cpp"


void update_drawing_options(State *state, GLFWwindow *window) {
  if (state->is_cursor_disabled) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  } else {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }
}


void process_input_continuous(GLFWwindow *window, State *state) {
  if (control_is_key_down(&state->control, GLFW_KEY_W)) {
    state->camera_active->move_front_back(1);
  }

  if (control_is_key_down(&state->control, GLFW_KEY_S)) {
    state->camera_active->move_front_back(-1);
  }

  if (control_is_key_down(&state->control, GLFW_KEY_A)) {
    state->camera_active->move_left_right(-1);
  }

  if (control_is_key_down(&state->control, GLFW_KEY_D)) {
    state->camera_active->move_left_right(1);
  }

  if (control_is_key_down(&state->control, GLFW_KEY_SPACE)) {
    state->camera_active->move_up_down(1);
  }

  if (control_is_key_down(&state->control, GLFW_KEY_LEFT_CONTROL)) {
    state->camera_active->move_up_down(-1);
  }
}


void process_input_transient(GLFWwindow *window, State *state) {
  if (control_is_key_now_down(&state->control, GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(window, true);
  }

  if (control_is_key_now_down(&state->control, GLFW_KEY_F)) {
    state->should_limit_fps = !state->should_limit_fps;
    update_drawing_options(state, window);
  }

  if (control_is_key_now_down(&state->control, GLFW_KEY_C)) {
    state->is_cursor_disabled = !state->is_cursor_disabled;
    update_drawing_options(state, window);
  }
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  State *state = (State*)glfwGetWindowUserPointer(window);
  state->window_width = width;
  state->window_height = height;
  log_info("Window is now: %d x %d", state->window_width, state->window_height);
  glViewport(0, 0, width, height);
}


void mouse_callback(GLFWwindow *window, real64 x, real64 y) {
  State *state = (State*)glfwGetWindowUserPointer(window);
  glm::vec2 mouse_offset = control_update_mouse(&state->control, x, y);
  state->camera_active->update_mouse(mouse_offset);
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  State *state = (State*)glfwGetWindowUserPointer(window);
  control_update_keys(&state->control, key, scancode, action, mods);
  process_input_transient(window, state);
}


void APIENTRY debug_message_callback(
  GLenum source, GLenum type, unsigned int id, GLenum severity, GLsizei length,
  const char *message, const void *userParam
) {
  // Ignore insignificant error/warning codes
  if (
    // Framebuffer detailed info: The driver allocated storage for
    // renderbuffer 1.
    id == 131169 ||
    // Program/shader state performance warning: Vertex shader in program 19
    // is being recompiled based on GL state.
    id == 131218 ||
    // Buffer detailed info: Buffer object 1522 (bound to
    // GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB (0), and GL_ARRAY_BUFFER_ARB,
    // usage hint is GL_DYNAMIC_DRAW) will use VIDEO memory as the source for
    // buffer object operations.
    id == 131185 ||
    // Texture state usage warning: The texture object (0) bound to texture
    // image unit 4 does not have a defined base level and cannot be used for
    // texture mapping.
    id == 131204
  ) {
    return;
  }

  log_warning("Debug message (%d): %s", id, message);

  switch (source) {
    case GL_DEBUG_SOURCE_API:
      log_warning("Source: API");
      break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
      log_warning("Source: Window System");
      break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
      log_warning("Source: Shader Compiler");
      break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
      log_warning("Source: Third Party");
      break;
    case GL_DEBUG_SOURCE_APPLICATION:
      log_warning("Source: Application");
      break;
    case GL_DEBUG_SOURCE_OTHER:
      log_warning("Source: Other");
      break;
  }

  switch (type) {
    case GL_DEBUG_TYPE_ERROR:
      log_warning("Type: Error");
      break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
      log_warning("Type: Deprecated Behaviour");
      break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
      log_warning("Type: Undefined Behaviour");
      break;
    case GL_DEBUG_TYPE_PORTABILITY:
      log_warning("Type: Portability");
      break;
    case GL_DEBUG_TYPE_PERFORMANCE:
      log_warning("Type: Performance");
      break;
    case GL_DEBUG_TYPE_MARKER:
      log_warning("Type: Marker");
      break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
      log_warning("Type: Push Group");
      break;
    case GL_DEBUG_TYPE_POP_GROUP:
      log_warning("Type: Pop Group");
      break;
    case GL_DEBUG_TYPE_OTHER:
      log_warning("Type: Other");
      break;
  }

  switch (severity) {
    case GL_DEBUG_SEVERITY_HIGH:
      log_warning("Severity: high");
      break;
    case GL_DEBUG_SEVERITY_MEDIUM:
      log_warning("Severity: medium");
      break;
    case GL_DEBUG_SEVERITY_LOW:
      log_warning("Severity: low");
      break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
      log_warning("Severity: notification");
      break;
  }

  log_newline();
}


Memory init_memory() {
  Memory memory;

  memory.state_memory_size = sizeof(State);
  log_info(
    "Allocating state memory: %.2fMB (%dB)",
    (real64)memory.state_memory_size / 1024 / 1024, memory.state_memory_size
  );
  memory.state_memory = (State *)malloc(memory.state_memory_size);
  memset(memory.state_memory, 0, memory.state_memory_size);

  memory.asset_memory_pool = memory_make_memory_pool(
    "assets", MEGABYTES(256)
  );

  memory.entity_memory_pool = memory_make_memory_pool(
    "entities", MEGABYTES(64)
  );

  memory.temp_memory_pool = memory_make_memory_pool(
    "temp", MEGABYTES(256)
  );

  return memory;
}


void init_state(Memory *memory, State *state) {
  new(&state->entities) Array<Entity>(&memory->entity_memory_pool, 512);
  new(&state->drawable_components) Array<DrawableComponent>(&memory->entity_memory_pool, 512);
  new(&state->light_components) Array<LightComponent>(&memory->entity_memory_pool, 512);
  new(&state->spatial_components) Array<SpatialComponent>(&memory->entity_memory_pool, 512);

  new(&state->lights) Array<EntityHandle>(&memory->entity_memory_pool, MAX_N_LIGHTS);
  new(&state->geese) Array<EntityHandle>(&memory->entity_memory_pool, 512);
  new(&state->spheres) Array<EntityHandle>(&memory->entity_memory_pool, 512);

  new(&state->entity_manager) EntityManager(
    &state->entities
  );
  new(&state->drawable_component_manager) DrawableComponentManager(
    &state->drawable_components
  );
  new(&state->light_component_manager) LightComponentManager(
    &state->light_components
  );
  new(&state->spatial_component_manager) SpatialComponentManager(
    &state->spatial_components
  );

  new(&state->shader_assets) Array<ShaderAsset>(&memory->asset_memory_pool, 512);
  new(&state->font_assets) Array<FontAsset>(&memory->asset_memory_pool, 512);
  new(&state->model_assets) Array<ModelAsset>(&memory->asset_memory_pool, 512);

  state->t = 0;
  state->dt = 0;

  state->is_cursor_disabled = true;
  state->should_limit_fps = false;
  state->background_color = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);

  new(&state->camera_main) Camera(CAMERA_PERSPECTIVE);
  state->camera_active = &state->camera_main;

  control_init(&state->control);
}


GLFWwindow* init_window(State *state) {
#ifdef USE_1440P
  state->window_width = 2560;
  state->window_height = 1440;
#else
  state->window_width = 1920;
  state->window_height = 1080;
#endif
  strcpy(state->window_title, "hi lol");

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
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

#if USE_OPENGL_DEBUG
  log_info("Using OpenGL debug context");
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

  GLFWwindow *window = glfwCreateWindow(
    state->window_width, state->window_height, state->window_title, nullptr, nullptr
  );
  if (!window) {
    log_error("Failed to create GLFW window");
    glfwTerminate();
    return nullptr;
  }
  glfwMakeContextCurrent(window);

  glfwSwapInterval(0);

#ifdef USE_1440P
  glfwSetWindowPos(window, 0, 0);
#else
  glfwSetWindowPos(window, 100, 100);
#endif

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    log_error("Failed to initialize GLAD");
    return nullptr;
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);

  /* glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); */

#if USE_OPENGL_DEBUG
  GLint flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

  if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(debug_message_callback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
  } else {
    log_fatal("Tried to initialise OpenGL debug output but couldn't");
  }
#endif

  glEnable(GL_CULL_FACE);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport(0, 0, state->window_width, state->window_height);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetKeyCallback(window, key_callback);

  update_drawing_options(state, window);

  glfwSetWindowUserPointer(window, state);

  return window;
}


void draw_text(
  State *state, const char* font_name, const char *str,
  real32 x, real32 y, real32 scale, glm::vec4 color
) {
  ShaderAsset *shader_asset = state->text_shader_asset;
  FontAsset *font_asset = FontAsset::get_by_name(&state->font_assets, font_name);

  glUseProgram(shader_asset->program);
  shader_asset->set_vec4("text_color", &color);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, font_asset->texture);

  glBindVertexArray(state->text_vao);
  glBindBuffer(GL_ARRAY_BUFFER, state->text_vbo);

  for (uint32 idx = 0; idx < strlen(str); idx++) {
    char c = str[idx];
    Character *character = font_asset->characters.get(c);

    real32 char_x = x + character->bearing.x * scale;
    real32 char_y = y - (character->size.y - character->bearing.y) * scale;

    real32 char_texture_w = (real32)character->size.x / font_asset->atlas_width;
    real32 char_texture_h = (real32)character->size.y / font_asset->atlas_height;

    real32 w = character->size.x * scale;
    real32 h = character->size.y * scale;

    x += (character->advance.x >> 6) * scale;
    y += (character->advance.y >> 6) * scale;

    if (w == 0 || h == 0) {
      // Skip glyphs with no pixels, like spaces.
      continue;
    }

    // TODO: Buffer vertices only once, use a matrix to transform the position.
    // NOTE: The correspondence between the y and texture y is the other way
    // around because the characters are upside down for some reason.
    real32 vertices[6][4] = {
      {char_x,     char_y + h,  character->texture_x,                  0},
      {char_x,     char_y,      character->texture_x,                  char_texture_h},
      {char_x + w, char_y,      character->texture_x + char_texture_w, char_texture_h},
      {char_x,     char_y + h,  character->texture_x,                  0},
      {char_x + w, char_y,      character->texture_x + char_texture_w, char_texture_h},
      {char_x + w, char_y + h,  character->texture_x + char_texture_w, 0}
    };

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}


void copy_scene_data_to_ubo(Memory *memory, State *state) {
  // TODO: Some of these things don't change every frame. Do we want to do a
  // separate block for those?
  ShaderCommon *shader_common = &state->shader_common;

  // NOTE: These are constant.
  shader_common->far_clip_dist = state->shadow_far_clip_dist;
  shader_common->shadow_light_idx = state->shadow_light_idx;

  // NOTE: These change every frame.
  memcpy(shader_common->shadow_transforms, state->shadow_transforms, sizeof(state->shadow_transforms));
  shader_common->view = state->camera_active->view;
  shader_common->projection = state->camera_active->projection;
  shader_common->camera_position = glm::vec4(state->camera_active->position, 1.0f);
  shader_common->exposure = state->camera_active->exposure;
  shader_common->t = (float)state->t;

  shader_common->n_lights = state->lights.get_size();
  for (uint32 idx = 0; idx < state->lights.get_size(); idx++) {
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
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void render_scene(
  Memory *memory, State *state, RenderPass render_pass, RenderMode render_mode
) {
  state->drawable_component_manager.draw_all(
    state->spatial_component_manager,
    render_pass, render_mode, state->entity_depth_shader_asset
  );

  // TODO: Move this into the entity system.
  if (render_pass == RENDERPASS_FORWARD) {
    glEnable(GL_BLEND);

    // TODO: Get rid of sprintf, it is #slow.
    const real32 row_height = 30.0f;
    char fps_text[128];
    sprintf(fps_text, "(fps %.2f)", state->last_fps);
    draw_text(
      state, "main-font", fps_text,
      15.0f, state->window_height - 35.0f - row_height * 0,
      1.0f, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
    sprintf(fps_text, "(t %f)", state->t);
    draw_text(
      state, "main-font", fps_text,
      15.0f, state->window_height - 35.0f - row_height * 1,
      1.0f, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
    sprintf(fps_text, "(dt %f)", state->dt);
    draw_text(
      state, "main-font", fps_text,
      15.0f, state->window_height - 35.0f - row_height * 2,
      1.0f, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
    sprintf(fps_text, "(should_limit_fps %d)", state->should_limit_fps);
    draw_text(
      state, "main-font", fps_text,
      15.0f, state->window_height - 35.0f - row_height * 3,
      1.0f, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
    sprintf(fps_text, "(exposure %.2f)", state->camera_active->exposure);
    draw_text(
      state, "main-font", fps_text,
      15.0f, state->window_height - 35.0f - row_height * 4,
      1.0f, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );
    sprintf(fps_text, "(oopses %d)", global_oopses);
    draw_text(
      state, "main-font", fps_text,
      15.0f, state->window_height - 35.0f - row_height * 5,
      1.0f, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );

    glDisable(GL_BLEND);
  }
}


void update_and_render(Memory *memory, State *state) {
  scene_update(memory, state);
  state->camera_active->update_matrices(state->window_width, state->window_height);
  copy_scene_data_to_ubo(memory, state);

  // Clear main framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render shadow map
  for (uint32 idx = 0; idx < state->n_shadow_framebuffers; idx++) {
    SpatialComponent *spatial_component =
      state->spatial_component_manager.get(*state->lights.get(idx));
    Camera::create_shadow_transforms(
      state->shadow_transforms, spatial_component->position,
      state->shadow_map_width, state->shadow_map_height,
      state->shadow_near_clip_dist, state->shadow_far_clip_dist
    );

    glBindFramebuffer(GL_FRAMEBUFFER, state->shadow_framebuffers[idx]);
    glViewport(0, 0, state->shadow_map_width, state->shadow_map_height);

    glClear(GL_DEPTH_BUFFER_BIT);

    state->shadow_light_idx = idx;
    copy_scene_data_to_ubo(memory, state);
    render_scene(
      memory, state, RENDERPASS_DEFERRED, RENDERMODE_DEPTH
    );

    glViewport(0, 0, state->window_width, state->window_height);
  }

  // Geometry pass
  glBindFramebuffer(GL_FRAMEBUFFER, state->g_buffer);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  render_scene(memory, state, RENDERPASS_DEFERRED, RENDERMODE_REGULAR);

  // Copy depth from geometry pass to lighting pass
  glBindFramebuffer(GL_READ_FRAMEBUFFER, state->g_buffer);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBlitFramebuffer(
    0, 0, state->window_width, state->window_height,
    0, 0, state->window_width, state->window_height,
    GL_DEPTH_BUFFER_BIT, GL_NEAREST
  );
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Lighting pass
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable(GL_DEPTH_TEST);
  glClearColor(
    state->background_color.r, state->background_color.g,
    state->background_color.b, state->background_color.a
  );
  glClear(GL_COLOR_BUFFER_BIT);
  render_scene(memory, state, RENDERPASS_LIGHTING, RENDERMODE_REGULAR);
  glEnable(GL_DEPTH_TEST);

  // Forward pass
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  render_scene(memory, state, RENDERPASS_FORWARD, RENDERMODE_REGULAR);
}


void init_shader_buffers(Memory *memory, State *state) {
  glGenBuffers(1, &state->ubo_shader_common);
  glBindBuffer(GL_UNIFORM_BUFFER, state->ubo_shader_common);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderCommon), NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  glBindBufferRange(GL_UNIFORM_BUFFER, 0, state->ubo_shader_common, 0, sizeof(ShaderCommon));
}


void init_shadow_buffers(Memory *memory, State *state) {
  state->shadow_map_width = 2048;
  state->shadow_map_height = 2048;
  state->shadow_near_clip_dist = 0.05f;
  state->shadow_far_clip_dist = 200.0f;
  state->n_shadow_framebuffers = state->lights.get_size();

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


void init_deferred_lighting_buffers(Memory *memory, State *state) {
  glGenFramebuffers(1, &state->g_buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, state->g_buffer);

  glGenTextures(1, &state->g_position_texture);
  glBindTexture(GL_TEXTURE_2D, state->g_position_texture);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA16F,
    state->window_width, state->window_height,
    0, GL_RGBA, GL_FLOAT, NULL
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, state->g_position_texture, 0
  );

  glGenTextures(1, &state->g_normal_texture);
  glBindTexture(GL_TEXTURE_2D, state->g_normal_texture);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA16F,
    state->window_width, state->window_height,
    0, GL_RGBA, GL_FLOAT, NULL
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, state->g_normal_texture, 0
  );

  glGenTextures(1, &state->g_albedo_texture);
  glBindTexture(GL_TEXTURE_2D, state->g_albedo_texture);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA,
    state->window_width, state->window_height,
    0, GL_RGBA, GL_UNSIGNED_BYTE, NULL
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, state->g_albedo_texture, 0
  );

  glGenTextures(1, &state->g_pbr_texture);
  glBindTexture(GL_TEXTURE_2D, state->g_pbr_texture);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA,
    state->window_width, state->window_height,
    0, GL_RGBA, GL_UNSIGNED_BYTE, NULL
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D, state->g_pbr_texture, 0
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
    GL_RENDERBUFFER, GL_DEPTH_COMPONENT, state->window_width, state->window_height
  );
  glFramebufferRenderbuffer(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth
  );

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    log_error("Framebuffer not complete!");
  }
}


void main_loop(GLFWwindow *window, Memory *memory, State *state) {
  std::chrono::steady_clock::time_point game_start = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point second_start = game_start;
  std::chrono::steady_clock::time_point frame_start = game_start;
  std::chrono::steady_clock::time_point last_frame_start = game_start;
  std::chrono::nanoseconds frame_duration = std::chrono::nanoseconds(6060606);
  uint32 n_frames_this_second = 0;

  while (!glfwWindowShouldClose(window)) {
    n_frames_this_second++;
    last_frame_start = frame_start;
    frame_start = std::chrono::steady_clock::now();
    std::chrono::duration<real64> time_since_second_start = frame_start - second_start;
    std::chrono::steady_clock::time_point frame_end_target_time = frame_start + frame_duration;

    std::chrono::duration<real64> time_since_last_frame = frame_start - last_frame_start;
    std::chrono::duration<real64> time_since_game_start = frame_start - game_start;
    state->t = std::chrono::duration_cast<std::chrono::duration<float>>(time_since_game_start).count();
    state->dt = std::chrono::duration_cast<std::chrono::duration<float>>(time_since_last_frame).count();

    if (time_since_second_start >= std::chrono::seconds(1)) {
      second_start = frame_start;
      state->last_fps = n_frames_this_second;
      n_frames_this_second = 0;
    }

    glfwPollEvents();
    process_input_continuous(window, state);
    update_and_render(memory, state);

    if (state->should_limit_fps) {
      std::this_thread::sleep_until(frame_end_target_time);
    }

    glfwSwapBuffers(window);
  }
}


void destroy_window() {
  glfwTerminate();
}


int main() {
  srand((uint32)time(NULL));
  Memory memory = init_memory();
  State *state = (State*)memory.state_memory;

  init_state(&memory, state);

  GLFWwindow *window = init_window(state);
  if (!window) {
    return -1;
  }

  init_deferred_lighting_buffers(&memory, state);
  init_shader_buffers(&memory, state);

  scene_resources_init_models(&memory, state);
  scene_resources_init_shaders(&memory, state);
  scene_resources_init_fonts(&memory, state);

  scene_init_objects(&memory, state);

  init_shadow_buffers(&memory, state);
  scene_init_screenquad(&memory, state);

  main_loop(window, &memory, state);
  destroy_window();
  return 0;
}
