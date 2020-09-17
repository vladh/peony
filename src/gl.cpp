#define _CRT_SECURE_NO_WARNINGS

#define USE_POSTPROCESSING false
#define USE_SHADOWS true
#define USE_ALPACA false

#include "gl.hpp"
#include "log.cpp"
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
  if (state->is_wireframe_on) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

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

  if (control_is_key_now_down(&state->control, GLFW_KEY_Q)) {
    state->is_wireframe_on = !state->is_wireframe_on;
    update_drawing_options(state, window);
  }

  if (control_is_key_now_down(&state->control, GLFW_KEY_E)) {
    state->should_draw_normals = !state->should_draw_normals;
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
  log_info("%d x %d", state->window_width, state->window_height);
  glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, real64 x, real64 y) {
  State *state = (State*)glfwGetWindowUserPointer(window);
  glm::vec2 mouse_offset = control_update_mouse(&state->control, x, y);
  camera_update_mouse(state->camera_active, mouse_offset);
  camera_update_matrices(state->camera_active, state->window_width, state->window_height);
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
  state->window_width = 1920;
  state->window_height = 1080;
  strcpy(state->window_title, "hi lol");

  array_init<Entity>(&memory->asset_memory_pool, &state->entities, 128);
  array_init<Entity*>(&memory->asset_memory_pool, &state->found_entities, 128);
  array_init<ShaderAsset>(&memory->asset_memory_pool, &state->shader_assets, 128);
  array_init<ModelAsset*>(&memory->asset_memory_pool, &state->model_assets, 128);
  array_init<Light>(&memory->asset_memory_pool, &state->lights, MAX_N_LIGHTS);

  state->is_wireframe_on = false;
  state->is_cursor_disabled = true;
  state->should_draw_normals = false;
  state->background_color = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);

  Light *light1 = array_push(&state->lights);
  light1->position = glm::vec3(0.0f, 1.0f, 0.0f);
  light1->direction = glm::vec3(0.0f, 0.0f, 0.0f);
  light1->ambient = glm::vec3(0.5f, 0.5f, 0.5f);
  light1->diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
  light1->specular = glm::vec3(1.0f, 1.0f, 1.0f);
  light1->attenuation_constant = 1.0f;
  light1->attenuation_linear = 0.09f;
  light1->attenuation_quadratic = 0.032f;

  Light *light2 = array_push(&state->lights);
  light2->position = glm::vec3(0.0f, 1.0f, 0.0f);
  light2->direction = glm::vec3(0.0f, 0.0f, 0.0f);
  light2->ambient = glm::vec3(0.0f, 0.0f, 0.0f);
  light2->diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
  light2->specular = glm::vec3(1.0f, 1.0f, 1.0f);
  light2->attenuation_constant = 1.0f;
  light2->attenuation_linear = 0.09f;
  light2->attenuation_quadratic = 0.032f;

  camera_init(&state->camera_main, CAMERA_PERSPECTIVE);
  camera_update_matrices(&state->camera_main, state->window_width, state->window_height);
  state->camera_active = &state->camera_main;

  control_init(&state->control);

  state->shadow_map_width = 2048;
  state->shadow_map_height = 2048;
  state->shadow_near_clip_dist = 0.1f;
  state->shadow_far_clip_dist = 25.0f;
  state->n_shadow_framebuffers = state->lights.size;
  state->shadow_light_idx = 0;
}

GLFWwindow* init_window(State *state) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
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

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    log_error("Failed to initialize GLAD");
    return nullptr;
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_MULTISAMPLE);

  glViewport(0, 0, state->window_width, state->window_height);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetKeyCallback(window, key_callback);

  update_drawing_options(state, window);

  glfwSetWindowUserPointer(window, state);

  return window;
}

void init_postprocessing_buffers(Memory *memory, State *state) {
  glGenFramebuffers(1, &state->postprocessing_framebuffer);
  glBindFramebuffer(GL_FRAMEBUFFER, state->postprocessing_framebuffer);

  uint32 color_texture;
  glGenTextures(1, &color_texture);
  glBindTexture(GL_TEXTURE_2D, color_texture);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGB, state->window_width, state->window_height,
    0, GL_RGB, GL_UNSIGNED_BYTE, NULL
  );
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
    color_texture, 0
  );

  uint32 rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);
  glRenderbufferStorage(
    GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, state->window_width, state->window_height
  );
  glFramebufferRenderbuffer(
    GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo
  );

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    log_error("Framebuffer is not complete");
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void init_shadow_buffers(Memory *memory, State *state) {
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

void set_render_mode(State *state, RenderMode render_mode) {
  state->render_mode = render_mode;
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
      // Only used in RENDERMODE_REGULAR
      shader_set_bool(shader_program, "should_draw_normals", state->should_draw_normals);
      shader_set_vec3(shader_program, "entity_color", &entity->color);
    } else if (state->render_mode == RENDERMODE_DEPTH) {
      // Only used when rendering to RENDERMODE_DEPTH
      shader_set_int(shader_program, "shadow_light_idx", state->shadow_light_idx);
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

void render_scene(Memory *memory, State *state) {
  camera_update_matrices(
    state->camera_active, state->window_width, state->window_height
  );

  // NOTE: Do we want to optimise this copying?
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

  draw_all_entities_with_name(memory, state, "axes");
  draw_all_entities_with_tag(memory, state, "light");
  draw_all_entities_with_name(memory, state, "floor");
  draw_all_entities_with_name(memory, state, "goose");
#if USE_ALPACA
  draw_all_entities_with_name(memory, state, "alpaca");
#endif
}

void update_and_render(Memory *memory, State *state) {
  // TODO: Clean this function up a bit.
  real64 t_now = glfwGetTime();
  state->dt = t_now - state->t;
  state->t = t_now;

  scene_update(memory, state);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glClearColor(
    state->background_color.r, state->background_color.g,
    state->background_color.b, state->background_color.a
  );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Render shadow map
#if USE_SHADOWS
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
    render_scene(memory, state);

    glViewport(0, 0, state->window_width, state->window_height);
  }
#endif

  // Render scene to postprocesisng buffer
#if USE_POSTPROCESSING
  glBindFramebuffer(GL_FRAMEBUFFER, state->postprocessing_framebuffer);

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  set_render_mode(state, RENDERMODE_REGULAR);
  render_scene(memory, state);
#endif

  // Render normal scene
#if !USE_POSTPROCESSING
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  set_render_mode(state, RENDERMODE_REGULAR);
  render_scene(memory, state);
#endif

  // Render postprocessing/shadow framebuffer onto quad
#if USE_POSTPROCESSING
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable(GL_DEPTH_TEST);
  glClearColor(1.0f, 0.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  draw_all_entities_with_name(memory, state, "screenquad");
  glEnable(GL_DEPTH_TEST);
#endif
}

void init_shader_buffers(Memory *memory, State *state) {
  glGenBuffers(1, &state->ubo_shader_common);
  glBindBuffer(GL_UNIFORM_BUFFER, state->ubo_shader_common);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderCommon), NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  glBindBufferRange(GL_UNIFORM_BUFFER, 0, state->ubo_shader_common, 0, sizeof(ShaderCommon));
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

#if USE_POSTPROCESSING
  init_postprocessing_buffers(&memory, state);
#endif

#if USE_SHADOWS
  init_shadow_buffers(&memory, state);
#endif

  init_shader_buffers(&memory, state);

  scene_resources_init_models(&memory, state);
  scene_resources_init_shaders(&memory, state);

  scene_init_objects(&memory, state);
  main_loop(window, &memory, state);
  destroy_window();
  return 0;
}
