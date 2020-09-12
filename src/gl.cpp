#define _CRT_SECURE_NO_WARNINGS

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>

#include "data.hpp"
#include "types.hpp"
#include "shader.hpp"
#include "models.hpp"
#include "asset.hpp"
#include "memory.hpp"
#include "control.hpp"
#include "log.hpp"
#include "util.hpp"
#include "camera.hpp"
#include "array.hpp"
#include "state.hpp"


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

void toggle_wireframe(State *state) {
  state->is_wireframe_on = !state->is_wireframe_on;
}

void toggle_cursor(State *state) {
  state->is_cursor_disabled = !state->is_cursor_disabled;
}

void process_input_continuous(GLFWwindow *window, State *state) {
  if (control_is_key_down(&state->control, GLFW_KEY_W)) {
    camera_move_front_back(&state->camera, 1);
  }

  if (control_is_key_down(&state->control, GLFW_KEY_S)) {
    camera_move_front_back(&state->camera, -1);
  }

  if (control_is_key_down(&state->control, GLFW_KEY_A)) {
    camera_move_left_right(&state->camera, -1);
  }

  if (control_is_key_down(&state->control, GLFW_KEY_D)) {
    camera_move_left_right(&state->camera, 1);
  }

  if (control_is_key_down(&state->control, GLFW_KEY_SPACE)) {
    camera_move_up_down(&state->camera, 1);
  }

  if (control_is_key_down(&state->control, GLFW_KEY_LEFT_CONTROL)) {
    camera_move_up_down(&state->camera, -1);
  }
}

void process_input_transient(GLFWwindow *window, State *state) {
  if (control_is_key_now_down(&state->control, GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(window, true);
  }

  if (control_is_key_now_down(&state->control, GLFW_KEY_Q)) {
    toggle_wireframe(state);
    update_drawing_options(state, window);
  }

  if (control_is_key_now_down(&state->control, GLFW_KEY_C)) {
    toggle_cursor(state);
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
  camera_update_mouse(&state->camera, mouse_offset);
  camera_update_matrices(&state->camera, state->window_width, state->window_height);
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
    "assets", MEGABYTES(512)
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
  array_init<Light>(&memory->asset_memory_pool, &state->lights, 32);

  state->is_wireframe_on = false;
  state->is_cursor_disabled = true;
  state->background_color = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);

  Light *light1 = array_push(&state->lights);
  light1->is_point_light = true;
  light1->position = glm::vec3(0.0f, 1.0f, 0.0f);
  light1->direction = glm::vec3(0.0f, 0.0f, 0.0f);
  light1->ambient = glm::vec3(0.5f, 0.5f, 0.5f);
  light1->diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
  light1->specular = glm::vec3(1.0f, 1.0f, 1.0f);
  light1->attenuation_constant = 1.0f;
  light1->attenuation_linear = 0.09f;
  light1->attenuation_quadratic = 0.032f;

  Light *light2 = array_push(&state->lights);
  light2->is_point_light = true;
  light2->position = glm::vec3(0.0f, 1.0f, 0.0f);
  light2->direction = glm::vec3(0.0f, 0.0f, 0.0f);
  light2->ambient = glm::vec3(0.0f, 0.0f, 0.0f);
  light2->diffuse = glm::vec3(0.5f, 0.5f, 0.5f);
  light2->specular = glm::vec3(1.0f, 1.0f, 1.0f);
  light2->attenuation_constant = 1.0f;
  light2->attenuation_linear = 0.09f;
  light2->attenuation_quadratic = 0.032f;

  camera_init(&state->camera);
  camera_update_matrices(&state->camera, state->window_width, state->window_height);
  control_init(&state->control);
}

GLFWwindow* init_window(State *state) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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

  glViewport(0, 0, state->window_width, state->window_height);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetKeyCallback(window, key_callback);

  update_drawing_options(state, window);

  glfwSetWindowUserPointer(window, state);

  return window;
}

void init_axes(Memory *memory, State *state) {
  ShaderAsset* shader_asset = shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    "axes", "src/shaders/axes.vert", "src/shaders/axes.frag"
  );

  const real32 axis_size = 20.0f;
  real32 axes_vertices[] = AXES_VERTICES;
  uint32 n_vertices = 6;

  ModelAsset *model_asset = models_make_asset_from_data(
    memory,
    array_push<ModelAsset*>(
      &state->model_assets,
      (ModelAsset*)memory_push_memory_to_pool(
        &memory->asset_memory_pool, sizeof(ModelAsset)
      )
    ),
    shader_asset,
    axes_vertices, n_vertices,
    nullptr, 0,
    "axes", "",
    GL_LINES
  );

  Entity *entity = entity_make(
    array_push<Entity>(&state->entities),
    "axes",
    ENTITY_MODEL,
    glm::vec3(0.0f, 0.1f, 0.0f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::angleAxis(
      glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
    )
  );

  entity_set_shader_asset(entity, shader_asset);
  entity_set_model_asset(entity, model_asset);
  entity_add_tag(entity, "axes");
}

void init_alpaca(Memory *memory, State *state) {
  ShaderAsset* shader_asset = shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    "alpaca", "src/shaders/alpaca.vert", "src/shaders/alpaca.frag"
  );

  real32 alpaca_vertices[] = ALPACA_VERTICES;
  uint32 n_vertices = 36;

  ModelAsset *model_asset = models_make_asset_from_data(
    memory,
    array_push<ModelAsset*>(
      &state->model_assets,
      (ModelAsset*)memory_push_memory_to_pool(
        &memory->asset_memory_pool, sizeof(ModelAsset)
      )
    ),
    shader_asset,
    alpaca_vertices, n_vertices,
    nullptr, 0,
    "alpaca", "resources/alpaca.jpg",
    GL_TRIANGLES
  );

  uint32 n_alpacas = 10;

  for (uint8 idx = 0; idx < n_alpacas; idx++) {
    real64 scale = util_random(1.0f, 1.4f);
    Entity *entity = entity_make(
      array_push<Entity>(&state->entities),
      "alpaca",
      ENTITY_MODEL,
      glm::vec3(
        util_random(-6.0f, 6.0f),
        util_random(1.0f, 6.0f),
        util_random(-6.0f, 6.0f)
      ),
      glm::vec3(scale, scale, scale),
      glm::angleAxis(
        glm::radians(-90.0f + (30.0f * idx)), glm::vec3(1.0f, 0.0f, 0.0f)
      )
    );

    entity_set_shader_asset(entity, shader_asset);
    entity_set_model_asset(entity, model_asset);
    entity_add_tag(entity, "alpaca");
  }
}

void init_floor(Memory *memory, State *state) {
  ShaderAsset *shader_asset = shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    "floor", "src/shaders/entity.vert", "src/shaders/entity.frag"
  );
  ModelAsset *model_asset = models_make_asset_from_file(
    memory,
    array_push<ModelAsset*>(
      &state->model_assets,
      (ModelAsset*)memory_push_memory_to_pool(
        &memory->asset_memory_pool, sizeof(ModelAsset)
      )
    ),
    shader_asset,
    "floor", "resources/", "cube.obj"
  );

  Entity *entity = entity_make(
    array_push<Entity>(&state->entities),
    "floor",
    ENTITY_MODEL,
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(150.0f, 0.1f, 150.0f),
    glm::angleAxis(
      glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
    )
  );

  entity_set_color(entity, glm::vec3(1.0f, 1.0f, 1.0f));
  entity_set_shader_asset(entity, shader_asset);
  entity_set_model_asset(entity, model_asset);
  entity_add_tag(entity, "floor");
}

void init_lights(Memory *memory, State *state) {
  ShaderAsset *shader_asset = shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    "light", "src/shaders/light.vert", "src/shaders/light.frag"
  );
  ModelAsset *model_asset = models_make_asset_from_file(
    memory,
    array_push<ModelAsset*>(
      &state->model_assets,
      (ModelAsset*)memory_push_memory_to_pool(
        &memory->asset_memory_pool, sizeof(ModelAsset)
      )
    ),
    shader_asset,
    "light", "resources/", "cube.obj"
  );

  Entity *light1 = entity_make(
    array_push<Entity>(&state->entities),
    "light1",
    ENTITY_MODEL,
    state->lights.items[0].position,
    glm::vec3(0.3f, 0.3f, 0.3f),
    glm::angleAxis(
      glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
    )
  );
  entity_set_shader_asset(light1, shader_asset);
  entity_set_model_asset(light1, model_asset);
  entity_add_tag(light1, "light");

  Entity *light2 = entity_make(
    array_push<Entity>(&state->entities),
    "light2",
    ENTITY_MODEL,
    state->lights.items[1].position,
    glm::vec3(0.3f, 0.3f, 0.3f),
    glm::angleAxis(
      glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
    )
  );
  entity_set_shader_asset(light2, shader_asset);
  entity_set_model_asset(light2, model_asset);
  entity_add_tag(light2, "light");
}

void init_geese(Memory *memory, State *state) {
  ShaderAsset *shader_asset = shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    "goose", "src/shaders/entity.vert", "src/shaders/entity.frag"
  );
  ModelAsset *model_asset = models_make_asset_from_file(
    memory,
    array_push<ModelAsset*>(
      &state->model_assets,
      (ModelAsset*)memory_push_memory_to_pool(
        &memory->asset_memory_pool, sizeof(ModelAsset)
      )
    ),
    shader_asset,
    "goose", "resources/", "miniGoose.fbx"
  );

  uint32 n_geese = 10;

  for (uint8 idx = 0; idx < n_geese; idx++) {
    real64 scale = util_random(0.2f, 0.4f);
    Entity *entity = entity_make(
      array_push<Entity>(&state->entities),
      "goose",
      ENTITY_MODEL,
      glm::vec3(
        util_random(-8.0f, 8.0f),
        0.1f,
        util_random(-8.0f, 8.0f)
      ),
      glm::vec3(scale, scale, scale),
      glm::angleAxis(
        glm::radians(-90.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
      )
    );

    entity_set_color(entity, glm::vec3(1.0f, 0.0f, 0.0f));
    entity_set_shader_asset(entity, shader_asset);
    entity_set_model_asset(entity, model_asset);
    entity_add_tag(entity, "goose");
  }
}

void init_objects(Memory *memory, State *state) {
  init_axes(memory, state);
  init_floor(memory, state);
  init_lights(memory, state);
  init_geese(memory, state);
#if 0
  init_alpaca(memory, state);
#endif
}

void draw_entity(State *state, Entity *entity) {
  if (entity->type == ENTITY_MODEL) {
    assert(entity->shader_asset);
    assert(entity->model_asset);

    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, entity->position);
    model_matrix = glm::scale(model_matrix, entity->scale);
    model_matrix = model_matrix * glm::toMat4(entity->rotation);

    uint32 shader_program = entity->shader_asset->shader.program;
    glUseProgram(shader_program);
    shader_set_mat4(shader_program, "model", &model_matrix);
    shader_set_mat4(shader_program, "view", &state->camera.view);
    shader_set_mat4(shader_program, "projection", &state->camera.projection);
    shader_set_float(shader_program, "t", (real32)state->t);
    shader_set_vec3(shader_program, "camera_position", &state->camera.position);
    shader_set_vec3(shader_program, "entity_color", &entity->color);

    shader_set_int(shader_program, "n_lights", state->lights.size);
    char uniform_name[128];

    for (uint32 idx = 0; idx < state->lights.size; idx++) {
      sprintf(uniform_name, "lights[%d].is_point_light", idx);
      shader_set_bool(shader_program, uniform_name, state->lights.items[idx].is_point_light);

      sprintf(uniform_name, "lights[%d].position", idx);
      shader_set_vec3(shader_program, uniform_name, &state->lights.items[idx].position);

      sprintf(uniform_name, "lights[%d].direction", idx);
      shader_set_vec3(shader_program, uniform_name, &state->lights.items[idx].direction);

      sprintf(uniform_name, "lights[%d].ambient", idx);
      shader_set_vec3(shader_program, uniform_name, &state->lights.items[idx].ambient);

      sprintf(uniform_name, "lights[%d].diffuse", idx);
      shader_set_vec3(shader_program, uniform_name, &state->lights.items[idx].diffuse);

      sprintf(uniform_name, "lights[%d].specular", idx);
      shader_set_vec3(shader_program, uniform_name, &state->lights.items[idx].specular);

      sprintf(uniform_name, "lights[%d].attenuation_constant", idx);
      shader_set_float(shader_program, uniform_name, state->lights.items[idx].attenuation_constant);

      sprintf(uniform_name, "lights[%d].attenuation_linear", idx);
      shader_set_float(shader_program, uniform_name, state->lights.items[idx].attenuation_linear);

      sprintf(uniform_name, "lights[%d].attenuation_quadratic", idx);
      shader_set_float(shader_program, uniform_name, state->lights.items[idx].attenuation_quadratic);
    }

    Model *model = &(entity->model_asset->model);
    models_draw_model(model, shader_program);
  } else {
    log_warning(
      "Do not know how to draw entity '%s' of type '%d'",
      entity->name, entity->type
    );
  }
}

void draw_background(Memory *memory, State *state) {
  glClearColor(
    state->background_color.r,
    state->background_color.g,
    state->background_color.b,
    state->background_color.a
  );
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void update_and_render_axes(Memory *memory, State *state) {
  entity_get_all_with_name(
    state->entities, "axes", &state->found_entities
  );

  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];
    draw_entity(state, entity);
  }
}

void update_and_render_lights(Memory *memory, State *state) {
  state->lights.items[0].position = glm::vec3(
    sin(state->t) * 3.0f,
    1.0f,
    0.0f
  );
  state->lights.items[1].position = glm::vec3(
    cos(state->t) * 3.0f,
    2.0f,
    sin(state->t) * 5.0f
  );

  entity_get_all_with_tag(
    state->entities, "light", &state->found_entities
  );

  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];
    entity->position = state->lights.items[idx].position;
    draw_entity(state, entity);
  }
}

void update_and_render_floor(Memory *memory, State *state) {
  entity_get_all_with_name(
    state->entities, "floor", &state->found_entities
  );

  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];
    draw_entity(state, entity);
  }
}

void update_and_render_geese(Memory *memory, State *state) {
  entity_get_all_with_name(
    state->entities, "goose", &state->found_entities
  );

  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];

    real32 period_offset = (real32)idx;
    real32 spin_speed_factor = 0.3f;
    real32 radius_offset = (2.0f + (idx * 1.0f));
    real32 pos_arg = ((real32)state->t * spin_speed_factor) + period_offset;
    real32 spin_deg_per_t = 90.0f;

    entity->position = glm::vec3(
      sin(pos_arg) * radius_offset,
      entity->position.y,
      cos(pos_arg) * radius_offset
    );
    entity->rotation *= glm::angleAxis(
      glm::radians(spin_deg_per_t * (real32)state->dt),
      glm::vec3(0.0f, 0.0f, 1.0f)
    );

    draw_entity(state, entity);
  }
}

void update_and_render_alpaca(Memory *memory, State *state) {
  entity_get_all_with_name(
    state->entities, "alpaca", &state->found_entities
  );

  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];
    entity->rotation *= glm::angleAxis(
      glm::radians(15.0f * (real32)state->dt),
      glm::vec3(1.0f, 0.0f, 0.0f)
    );
    draw_entity(state, entity);
  }
}

void update_and_render(Memory *memory, State *state) {
  real64 t_now = glfwGetTime();
  state->dt = t_now - state->t;
  state->t = t_now;
  camera_update_matrices(&state->camera, state->window_width, state->window_height);
  draw_background(memory, state);
  update_and_render_axes(memory, state);
  update_and_render_floor(memory, state);
  update_and_render_lights(memory, state);
  update_and_render_geese(memory, state);
#if 0
  update_and_render_alpaca(memory, state);
#endif
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
  init_objects(&memory, state);
  main_loop(window, &memory, state);
  destroy_window();
  return 0;
}
