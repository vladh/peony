#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4100)
#pragma warning(disable : 4127)
#pragma warning(disable : 4201)

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

bool32 is_key_down(State *state, int key) {
  return state->key_states[key];
}

bool32 is_key_up(State *state, int key) {
  return !state->key_states[key];
}

bool32 is_key_now_down(State *state, int key) {
  return state->key_states[key] && !state->prev_key_states[key];
}

bool32 is_key_now_up(State *state, int key) {
  return !state->key_states[key] && state->prev_key_states[key];
}

void process_input_continuous(GLFWwindow *window, State *state) {
  if (is_key_down(state, GLFW_KEY_W)) {
    camera_move_front_back(&state->camera, 1);
  }

  if (is_key_down(state, GLFW_KEY_S)) {
    camera_move_front_back(&state->camera, -1);
  }

  if (is_key_down(state, GLFW_KEY_A)) {
    camera_move_left_right(&state->camera, -1);
  }

  if (is_key_down(state, GLFW_KEY_D)) {
    camera_move_left_right(&state->camera, 1);
  }

  if (is_key_down(state, GLFW_KEY_SPACE)) {
    camera_move_up_down(&state->camera, 1);
  }

  if (is_key_down(state, GLFW_KEY_LEFT_CONTROL)) {
    camera_move_up_down(&state->camera, -1);
  }
}

void process_input_transient(GLFWwindow *window, State *state) {
  if (is_key_now_down(state, GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(window, true);
  }

  if (is_key_now_down(state, GLFW_KEY_Q)) {
    toggle_wireframe(state);
    update_drawing_options(state, window);
  }

  if (is_key_now_down(state, GLFW_KEY_C)) {
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
  glm::vec2 mouse_offset = control_update(&state->control, x, y);
  camera_update_mouse(&state->camera, mouse_offset);
  camera_update_matrices(&state->camera, state->window_width, state->window_height);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  State *state = (State*)glfwGetWindowUserPointer(window);

  state->prev_key_states[key] = state->key_states[key];
  if (action == GLFW_PRESS) {
    state->key_states[key] = true;
  } else if (action == GLFW_RELEASE) {
    state->key_states[key] = false;
  }

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

  real32 test_vertices[] = {
    // positions          colors             texture coords
     /* 0.5f,  0.5f,  0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f, // top right */
     /* 0.5f, -0.5f,  0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, // bottom right */
    /* -0.5f, -0.5f,  0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // bottom left */
    /* -0.5f,  0.5f,  0.0f,  1.0f, 0.0f, 1.0f,  0.0f, 1.0f, // top left */

    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f, 1.0f, 0.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 1.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  1.0f, 0.0f, 1.0f,  0.0f, 1.0f
  };
  uint32 test_indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3 // second triangle
  };
  glm::vec3 test_cube_positions[] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
  };
  memcpy(state->test_vertices, test_vertices, sizeof(test_vertices));
  memcpy(state->test_indices, test_indices, sizeof(test_indices));
  memcpy(state->test_cube_positions, test_cube_positions, sizeof(test_cube_positions));

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

  for (uint32 idx = 0; idx < len(state->key_states); idx++) {
    state->key_states[idx] = false;
    state->prev_key_states[idx] = true;
  }
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

void init_alpaca(Memory *memory, State *state) {
  ShaderAsset* shader_asset = shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    "alpaca", "src/alpaca.vert", "src/alpaca.frag"
  );

  uint32 vbo, vao, ebo;
  glGenVertexArrays(1, &vao);
  glGenBuffers(1, &vbo);
  glGenBuffers(1, &ebo);
  glBindVertexArray(vao);

  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(
    GL_ARRAY_BUFFER, sizeof(state->test_vertices), state->test_vertices, GL_STATIC_DRAW
  );

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
  glBufferData(
    GL_ELEMENT_ARRAY_BUFFER, sizeof(state->test_indices), state->test_indices, GL_STATIC_DRAW
  );

  uint32 location;

  location = glGetAttribLocation(shader_asset->shader.program, "position");
  glEnableVertexAttribArray(location);
  glVertexAttribPointer(
    location, 3, GL_FLOAT, false, 8 * sizeof(real32), (void*)0
  );

  location = glGetAttribLocation(shader_asset->shader.program, "color");
  glEnableVertexAttribArray(location);
  glVertexAttribPointer(
    location, 3, GL_FLOAT, false, 8 * sizeof(real32), (void*)(3 * sizeof(real32))
  );

  location = glGetAttribLocation(shader_asset->shader.program, "tex_coords");
  glEnableVertexAttribArray(location);
  glVertexAttribPointer(
    location, 2, GL_FLOAT, false, 8 * sizeof(real32), (void*)(6 * sizeof(real32))
  );

  glBindVertexArray(0);

  state->test_texture = models_load_texture_from_file(
    "resources/", "alpaca.jpg"
  );

  state->test_vao = vao;
}

void init_geese(Memory *memory, State *state) {
  ShaderAsset *shader_asset = shader_make_asset(
    array_push<ShaderAsset>(&state->shader_assets),
    "goose", "src/goose.vert", "src/goose.frag"
  );
  ModelAsset *model_asset = models_make_asset(
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
        util_random(-8.0f, 8.0f),
        util_random(-8.0f, 8.0f)
      ),
      glm::vec3(scale, scale, scale),
      glm::angleAxis(
        glm::radians(-90.0f + (30.0f * idx)), glm::vec3(1.0f, 0.0f, 0.0f)
      )
    );

    entity_set_shader_asset(entity, shader_asset);
    entity_set_model_asset(entity, model_asset);
    entity_add_tag(entity, "goose");
  }
}

#if 0
void init_backpack(Memory *memory, State *state) {
  shader_make_asset(
    (ShaderAsset*)array_push<ShaderAsset>(&state->shader_assets),
    "backpack", "src/backpack.vert", "src/backpack.frag"
  );
}

void draw_backpack(State *state, glm::mat4 view, glm::mat4 projection) {
  ShaderAsset *backpack_shader_asset = asset_get_shader_asset_by_name(
    state->shader_assets, state->n_shader_assets, "backpack"
  );
  glUseProgram(backpack_shader_asset->shader.program);

  glUniformMatrix4fv(
    glGetUniformLocation(backpack_shader_asset->shader.program, "view"),
    1, GL_FALSE, glm::value_ptr(view)
  );

  glUniformMatrix4fv(
    glGetUniformLocation(backpack_shader_asset->shader.program, "projection"),
    1, GL_FALSE, glm::value_ptr(projection)
  );

  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
  model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
  glUniformMatrix4fv(
    glGetUniformLocation(backpack_shader_asset->shader.program, "model"),
    1, GL_FALSE, glm::value_ptr(model)
  );

  ModelAsset *backpack_model_asset = asset_get_model_asset_by_name(
    state->model_assets, state->n_model_assets, "backpack"
  );
  models_draw_model(&backpack_model_asset->model, backpack_shader_asset->shader.program);
}
#endif

void init_objects(Memory *memory, State *state) {
  init_geese(memory, state);
  init_alpaca(memory, state);
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

void draw_alpaca(Memory *memory, State *state) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, state->test_texture);

  glm::mat4 model = glm::mat4(1.0f);

  ShaderAsset *alpaca_shader_asset = asset_get_shader_asset_by_name(
    &state->shader_assets, "alpaca"
  );
  glUseProgram(alpaca_shader_asset->shader.program);

  glUniform1f(
    glGetUniformLocation(alpaca_shader_asset->shader.program, "t"),
    (real32)state->t
  );

  glUniformMatrix4fv(
    glGetUniformLocation(alpaca_shader_asset->shader.program, "view"),
    1, GL_FALSE, glm::value_ptr(state->camera.view)
  );

  glUniformMatrix4fv(
    glGetUniformLocation(alpaca_shader_asset->shader.program, "projection"),
    1, GL_FALSE, glm::value_ptr(state->camera.projection)
  );

  glBindVertexArray(state->test_vao);
  for (uint8 i = 0; i < 10; i++) {
    real32 model_angle = 20.0f * i;
    model = glm::translate(glm::mat4(1.0f), state->test_cube_positions[i]);
    model = glm::rotate(model, (real32)state->t * glm::radians(model_angle), glm::vec3(1.0f, 0.3f, 0.5f));
    glUniformMatrix4fv(
      glGetUniformLocation(alpaca_shader_asset->shader.program, "model"),
      1, GL_FALSE, glm::value_ptr(model)
    );

    glDrawArrays(GL_TRIANGLES, 0, 36);
  }
  glBindVertexArray(0);
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
  draw_alpaca(memory, state);

  entity_get_all_with_tag(
    state->entities, "goose", &state->found_entities
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
