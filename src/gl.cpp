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
    "Allocating state memory: %d MB",
    memory.state_memory_size / 1024 / 1024
  );
  memory.state_memory = (State *)malloc(memory.state_memory_size);
  memset(memory.state_memory, 0, memory.state_memory_size);

  memory.asset_memory_pool = memory_make_memory_pool(
    "assets", megabytes(512)
  );

  log_newline();

  return memory;
}

void init_state(Memory *memory, State *state) {
  state->window_width = 1920;
  state->window_height = 1080;
  strcpy(state->window_title, "hi lol");

  log_info("Pushing memory for entities");
  state->entities.size = 0;
  state->entities.max_size = 128;
  state->entities.items = (Entity*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(Entity) * state->entities.max_size
  );

  log_info("Pushing memory for found entities");
  state->found_entities.size = 0;
  state->found_entities.max_size = 128;
  state->found_entities.items = (Entity**)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(Entity*) * state->found_entities.max_size
  );

  log_info("Pushing memory for shader assets");
  state->shader_assets.size = 0;
  state->shader_assets.max_size = 128;
  state->shader_assets.items = (ShaderAsset*)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(ShaderAsset) * state->shader_assets.max_size
  );

  log_info("Pushing memory for model assets");
  state->model_assets.size = 0;
  state->model_assets.max_size = 32;
  state->model_assets.items = (ModelAsset**)memory_push_memory_to_pool(
    &memory->asset_memory_pool, sizeof(ModelAsset*) * state->model_assets.max_size
  );

  camera_init(&state->camera);
  camera_update_matrices(&state->camera, state->window_width, state->window_height);

  control_init(&state->control);

  state->is_wireframe_on = false;
  state->is_cursor_disabled = true;
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
    "axes", "src/axes.vert", "src/axes.frag"
  );

  const real32 axis_size = 20.0f;
  real32 axes_vertices[] = {
    // position                       normal             texture_coord
    0.0f,      0.0f,      0.0f,       1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    axis_size, 0.0f,      0.0f,       1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    0.0f,      0.0f,      0.0f,       0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    0.0f,      axis_size, 0.0f,       0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
    0.0f,      0.0f,      0.0f,       0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
    0.0f,      0.0f,      axis_size,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f
  };
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
    "alpaca", "src/alpaca.vert", "src/alpaca.frag"
  );

  real32 alpaca_vertices[] = {
    // position           normal             texture_coords
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 0.0f,  0.0f, 1.0f
  };
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
    "floor", "src/floor.vert", "src/floor.frag"
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
    glm::vec3(50.0f, 0.1f, 50.0f),
    glm::angleAxis(
      glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f)
    )
  );

  entity_set_shader_asset(entity, shader_asset);
  entity_set_model_asset(entity, model_asset);
  entity_add_tag(entity, "floor");
}

void init_geese(Memory *memory, State *state) {
  ShaderAsset *shader_asset = shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    "goose", "src/goose.vert", "src/goose.frag"
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
        /* util_random(1.0f, 8.0f), */
        util_random(-8.0f, 8.0f)
      ),
      glm::vec3(scale, scale, scale),
      glm::angleAxis(
        glm::radians(-90.0f),
        glm::vec3(1.0f, 0.0f, 0.0f)
      )
    );

    entity_set_shader_asset(entity, shader_asset);
    entity_set_model_asset(entity, model_asset);
    entity_add_tag(entity, "goose");
  }
}

void init_objects(Memory *memory, State *state) {
  init_axes(memory, state);
  init_floor(memory, state);
  init_geese(memory, state);
#if 0
  init_alpaca(memory, state);
#endif
}

void draw_entity(State *state, Entity *entity) {
  if (entity->type == ENTITY_MODEL) {
    assert(entity->shader_asset);
    assert(entity->model_asset);

    // Shader
    uint32 shader_program = entity->shader_asset->shader.program;
    glUseProgram(shader_program);

    glUniform1f(
      glGetUniformLocation(shader_program, "t"),
      (real32)state->t
    );

    glUniformMatrix4fv(
      glGetUniformLocation(shader_program, "view"),
      1, GL_FALSE, glm::value_ptr(state->camera.view)
    );

    glUniformMatrix4fv(
      glGetUniformLocation(shader_program, "projection"),
      1, GL_FALSE, glm::value_ptr(state->camera.projection)
    );

    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, entity->position);
    model_matrix = glm::scale(model_matrix, entity->scale);
    model_matrix = model_matrix * glm::toMat4(entity->rotation);
    glUniformMatrix4fv(
      glGetUniformLocation(shader_program, "model"),
      1, GL_FALSE, glm::value_ptr(model_matrix)
    );

    // Model
    Model *model = &(entity->model_asset->model);
    models_draw_model(model, shader_program);
  } else {
    log_warning(
      "Do not know how to draw entity '%s' of type '%d'",
      entity->name, entity->type
    );
  }
}

void draw_background() {
  glClearColor(0.180f, 0.204f, 0.251f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void update_and_render(Memory *memory, State *state) {
  real64 t_now = glfwGetTime();
  state->dt = t_now - state->t;
  state->t = t_now;

  camera_update_matrices(&state->camera, state->window_width, state->window_height);

  draw_background();

  // Axes
  entity_get_all_with_name(
    state->entities, "axes", &state->found_entities
  );

  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];
    draw_entity(state, entity);
  }

  // Floor
  entity_get_all_with_name(
    state->entities, "floor", &state->found_entities
  );

  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];
    draw_entity(state, entity);
  }

  // Geese
  entity_get_all_with_name(
    state->entities, "goose", &state->found_entities
  );

  for (uint32 idx = 0; idx < state->found_entities.size; idx++) {
    Entity *entity = state->found_entities.items[idx];
    entity->rotation *= glm::angleAxis(
      glm::radians(30.0f * (real32)state->dt),
      glm::vec3(0.0f, 0.0f, 1.0f)
    );
    draw_entity(state, entity);
  }

#if 0
  // Alpaca
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
