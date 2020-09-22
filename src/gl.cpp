#define _CRT_SECURE_NO_WARNINGS

#define USE_ALPACA false
#define USE_AXES false
#define SHOULD_LIMIT_FRAMES false

#include "gl.hpp"
#include "log.cpp"
#include "font.cpp"
#include "shader.cpp"
#include "util.cpp"
#include "camera.cpp"
#include "asset.cpp"
#include "memory.cpp"
#include "control.cpp"
#include "entity.cpp"
#include "array.cpp"
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
    camera_move_front_back(state->camera_active, 1);
  }

  if (control_is_key_down(&state->control, GLFW_KEY_S)) {
    camera_move_front_back(state->camera_active, -1);
  }

  if (control_is_key_down(&state->control, GLFW_KEY_A)) {
    camera_move_left_right(state->camera_active, -1);
  }

  if (control_is_key_down(&state->control, GLFW_KEY_D)) {
    camera_move_left_right(state->camera_active, 1);
  }

  if (control_is_key_down(&state->control, GLFW_KEY_SPACE)) {
    camera_move_up_down(state->camera_active, 1);
  }

  if (control_is_key_down(&state->control, GLFW_KEY_LEFT_CONTROL)) {
    camera_move_up_down(state->camera_active, -1);
  }
}

void process_input_transient(GLFWwindow *window, State *state) {
  if (control_is_key_now_down(&state->control, GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(window, true);
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
  log_info("%d x %d", state->window_width, state->window_height);
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, real64 x, real64 y) {
  State *state = (State*)glfwGetWindowUserPointer(window);
  glm::vec2 mouse_offset = control_update_mouse(&state->control, x, y);
  camera_update_mouse(state->camera_active, mouse_offset);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  State *state = (State*)glfwGetWindowUserPointer(window);
  control_update_keys(&state->control, key, scancode, action, mods);
  process_input_transient(window, state);
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

  memory.temp_memory_pool = memory_make_memory_pool(
    "temp", MEGABYTES(256)
  );

  return memory;
}

void init_state(Memory *memory, State *state) {
  array_init<Entity>(&memory->asset_memory_pool, &state->entities, 128);
  array_init<Entity*>(&memory->asset_memory_pool, &state->found_entities, 128);
  array_init<ShaderAsset>(&memory->asset_memory_pool, &state->shader_assets, 128);
  array_init<FontAsset>(&memory->asset_memory_pool, &state->font_assets, 128);
  array_init<ModelAsset>(&memory->asset_memory_pool, &state->model_assets, 128);
  array_init<Light>(&memory->asset_memory_pool, &state->lights, MAX_N_LIGHTS);

  state->t = 0;
  state->dt = 0;
  state->target_fps = 165.0f;
  state->target_frame_duration_s = 1 / state->target_fps;

  state->is_cursor_disabled = true;
  state->background_color = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);

  camera_init(&state->camera_main, CAMERA_PERSPECTIVE);
  state->camera_active = &state->camera_main;

  control_init(&state->control);
}

GLFWwindow* init_window(State *state) {
  state->window_width = 1920;
  state->window_height = 1080;
  strcpy(state->window_title, "hi lol");

  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

#if defined(__APPLE__)
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
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
  glfwSetWindowPos(window, 100, 100);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    log_error("Failed to initialize GLAD");
    return nullptr;
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);

  // NOTE: This breaks the alpaca cubes. It's probably the cubes'
  // fault, but we should check that!
  /* glEnable(GL_CULL_FACE); */

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport(0, 0, state->window_width, state->window_height);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetKeyCallback(window, key_callback);

  update_drawing_options(state, window);

  glfwSetWindowUserPointer(window, state);

  return window;
}

void set_render_mode(State *state, RenderMode render_mode) {
  state->render_mode = render_mode;
}

void draw_text(
  State *state, const char* font_name, const char *str,
  real32 x, real32 y, real32 scale, glm::vec4 color
) {
  ShaderAsset *shader_asset = asset_get_shader_asset_by_name(
    &state->shader_assets, "text"
  );
  uint32 shader_program = shader_asset->shader.program;

  FontAsset *font_asset = asset_get_font_asset_by_name(&state->font_assets, font_name);
  Font *font = &font_asset->font;

  glUseProgram(shader_program);
  shader_set_vec4(shader_program, "text_color", &color);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, font->texture);

  glBindVertexArray(state->text_vao);
  glBindBuffer(GL_ARRAY_BUFFER, state->text_vbo);

  for (uint32 idx = 0; idx < strlen(str); idx++) {
    char c = str[idx];
    Character *character = &font->characters.items[c];

    real32 char_x = x + character->bearing.x * scale;
    real32 char_y = y - (character->size.y - character->bearing.y) * scale;

    real32 char_texture_w = (real32)character->size.x / font->atlas_width;
    real32 char_texture_h = (real32)character->size.y / font->atlas_height;

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
  glBindTexture(GL_TEXTURE_2D, 0);
}

void draw_entity(State *state, Entity *entity) {
  if (entity->type == ENTITY_MODEL) {
    assert(entity->model_asset);

    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, entity->position);
    model_matrix = glm::scale(model_matrix, entity->scale);
    model_matrix = model_matrix * glm::toMat4(entity->rotation);

    ShaderAsset *shader_asset;
    if (entity->shader_asset) {
      shader_asset = entity->shader_asset;
    } else {
      if (state->render_mode == RENDERMODE_REGULAR) {
        shader_asset = state->entity_shader_asset;
      } else if (state->render_mode == RENDERMODE_DEPTH) {
        shader_asset = state->entity_depth_shader_asset;
      } else {
        log_error("Could not find shader asset for entity %s", entity->name);
        shader_asset = nullptr;
      }
    }

    uint32 shader_program = shader_asset->shader.program;
    glUseProgram(shader_program);
    shader_set_mat4(shader_program, "model", &model_matrix);

    if (state->render_mode == RENDERMODE_REGULAR) {

    } else if (state->render_mode == RENDERMODE_DEPTH) {
      shader_set_int(shader_program, "shadow_light_idx", state->shadow_light_idx);
    }

    // TODO: Do this in a better way?
    if (strcmp(shader_asset->info.name, "screenquad") == 0) {
      shader_set_float(shader_program, "exposure", state->camera_active->exposure);
    }

    models_draw_model(&entity->model_asset->model, shader_program);
  } else {
    log_warning(
      "Do not know how to draw entity '%s' of type '%d'",
      entity->name, entity->type
    );
  }
}

void draw_all_entities_with_name(Memory *memory, State *state, const char* name) {
  entity_get_all_with_name(
    state->entities, name, &state->found_entities
  );
  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];
    draw_entity(state, entity);
  }
}

void draw_all_entities_with_tag(Memory *memory, State *state, const char* tag) {
  entity_get_all_with_tag(
    state->entities, tag, &state->found_entities
  );
  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];
    draw_entity(state, entity);
  }
}

void copy_scene_data_to_ubo(Memory *memory, State *state) {
  // NOTE: Do we want to optimise this copying?
  // TODO: Some of these things don't change every frame. Do we want to do a
  // separate block for those?
  // TODO: Think about converting everything to vec4.
  ShaderCommon *shader_common = &state->shader_common;
  shader_common->view = state->camera_active->view;
  shader_common->projection = state->camera_active->projection;
  memcpy(shader_common->shadow_transforms, state->shadow_transforms, sizeof(state->shadow_transforms));
  shader_common->camera_position = state->camera_active->position;
  shader_common->t = (float)state->t;
  shader_common->far_clip_dist = state->shadow_far_clip_dist;
  shader_common->n_lights = state->lights.size;
  memcpy(shader_common->lights, state->lights.items, sizeof(Light) * state->lights.size);

  glBindBuffer(GL_UNIFORM_BUFFER, state->ubo_shader_common);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderCommon), shader_common);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void render_scene(Memory *memory, State *state) {
  draw_all_entities_with_name(memory, state, "floor");
  draw_all_entities_with_name(memory, state, "goose");
  draw_all_entities_with_name(memory, state, "temple");
#if USE_AXES
  draw_all_entities_with_name(memory, state, "axes");
#endif
#if USE_ALPACA
  draw_all_entities_with_name(memory, state, "alpaca");
#endif
}

void render_scene_forward(Memory *memory, State *state) {
  draw_all_entities_with_tag(memory, state, "light");

  glEnable(GL_BLEND);

  // TODO: Get rid of sprintf.
  const real32 row_height = 30.0f;
  char fps_text[128];
  sprintf(fps_text, "(fps %.2f)", state->last_fps);
  draw_text(
    state, "main-font", fps_text,
    15.0f, state->window_height - 35.0f,
    1.0f, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
  );
  sprintf(fps_text, "(exposure %.2f)", state->camera_active->exposure);
  draw_text(
    state, "main-font", fps_text,
    15.0f, state->window_height - 35.0f - row_height,
    1.0f, glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
  );

  glDisable(GL_BLEND);
}

void update_and_render(Memory *memory, State *state) {
  real64 t_start = glfwGetTime();
  state->t = t_start;

  scene_update(memory, state);
  camera_update_matrices(
    state->camera_active, state->window_width, state->window_height
  );

  // Clear main framebuffer
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render shadow map
  for (uint32 idx = 0; idx < state->n_shadow_framebuffers; idx++) {
    camera_create_shadow_transforms(
      state->shadow_transforms, state->lights.items[idx].position,
      state->shadow_map_width, state->shadow_map_height,
      state->shadow_near_clip_dist, state->shadow_far_clip_dist
    );
    state->shadow_light_idx = idx;

    glBindFramebuffer(GL_FRAMEBUFFER, state->shadow_framebuffers[idx]);
    glViewport(0, 0, state->shadow_map_width, state->shadow_map_height);

    glClear(GL_DEPTH_BUFFER_BIT);

    set_render_mode(state, RENDERMODE_DEPTH);
    copy_scene_data_to_ubo(memory, state);
    render_scene(memory, state);

    glViewport(0, 0, state->window_width, state->window_height);
  }

  // Geometry pass
  glBindFramebuffer(GL_FRAMEBUFFER, state->g_buffer);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  set_render_mode(state, RENDERMODE_REGULAR);
  copy_scene_data_to_ubo(memory, state);
  render_scene(memory, state);

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
  draw_all_entities_with_name(memory, state, "screenquad");
  glEnable(GL_DEPTH_TEST);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  copy_scene_data_to_ubo(memory, state);
  render_scene_forward(memory, state);

#if SHOULD_LIMIT_FRAMES
  real64 t_end_prewait = glfwGetTime();
  real64 dt_prewait = t_end_prewait - t_start;

  real64 time_to_wait = state->target_frame_duration_s - dt_prewait;
  if (time_to_wait < 0) {
    log_warning("Frame took too long! %.6fs", state->dt);
  } else {
    util_sleep(time_to_wait);
  }
#endif

  real64 t_end = glfwGetTime();
  state->dt = t_end - t_start;
  state->last_fps = 1.0f / state->dt;
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
  state->shadow_near_clip_dist = 0.1f;
  state->shadow_far_clip_dist = 75.0f;
  state->n_shadow_framebuffers = state->lights.size;
  state->shadow_light_idx = 0;

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
  while(!glfwWindowShouldClose(window)) {
    process_input_continuous(window, state);
    update_and_render(memory, state);
    glfwSwapBuffers(window);
    glfwPollEvents();
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

  scene_init_lights(&memory, state);

  init_deferred_lighting_buffers(&memory, state);
  init_shadow_buffers(&memory, state);
  init_shader_buffers(&memory, state);

  scene_resources_init_models(&memory, state);
  scene_resources_init_shaders(&memory, state);
  scene_resources_init_fonts(&memory, state);

  scene_init_objects(&memory, state);
  main_loop(window, &memory, state);
  destroy_window();
  return 0;
}
