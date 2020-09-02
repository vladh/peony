#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4201)
#pragma warning(disable : 4127)

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "gl.hpp"
#include "types.hpp"
#include "log.hpp"
#include "util.hpp"
#include "shader.hpp"
#include "camera.hpp"
#include "models.hpp"


global_variable bool32 key_states[1024] = {false};
global_variable bool32 prev_key_states[1024] = {true};


void toggle_wireframe(State *state) {
  if (state->is_wireframe_on) {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  }
  state->is_wireframe_on = !state->is_wireframe_on;
}

bool32 is_key_down(int key) {
  return key_states[key];
}

bool32 is_key_up(int key) {
  return !key_states[key];
}

bool32 is_key_now_down(int key) {
  return key_states[key] && !prev_key_states[key];
}

bool32 is_key_now_up(int key) {
  return !key_states[key] && prev_key_states[key];
}

void process_input_continuous(GLFWwindow *window, State *state) {
  if (is_key_down(GLFW_KEY_W)) {
    camera_move_front_back(state, 1);
  }

  if (is_key_down(GLFW_KEY_S)) {
    camera_move_front_back(state, -1);
  }

  if (is_key_down(GLFW_KEY_A)) {
    camera_move_left_right(state, -1);
  }

  if (is_key_down(GLFW_KEY_D)) {
    camera_move_left_right(state, 1);
  }

  if (is_key_down(GLFW_KEY_SPACE)) {
    camera_move_up_down(state, 1);
  }

  if (is_key_down(GLFW_KEY_LEFT_CONTROL)) {
    camera_move_up_down(state, -1);
  }
}

void process_input_transient(GLFWwindow *window, State *state) {
  if (is_key_now_down(GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(window, true);
  }

  if (is_key_now_down(GLFW_KEY_Q)) {
    toggle_wireframe(state);
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
  camera_update_mouse(state, x, y);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  State *state = (State*)glfwGetWindowUserPointer(window);

  prev_key_states[key] = key_states[key];
  if (action == GLFW_PRESS) {
    key_states[key] = true;
  } else if (action == GLFW_RELEASE) {
    key_states[key] = false;
  }

  process_input_transient(window, state);
}

State* init_state() {
  size_t state_size = sizeof(State);
  State *state = (State*)malloc(state_size);
  log_info("Allocating %dMB", state_size / 1024 / 1024);
  log_newline();
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
  glm::vec3 cube_positions[] = {
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
  memcpy(state->cube_positions, cube_positions, sizeof(cube_positions));

  state->yaw = -90.0f;
  state->pitch = 0.0f;

  state->camera_pos = glm::vec3(0.0f, 0.0f, 3.0f);
  state->camera_front = glm::vec3(0.0f, 0.0f, 0.0f);
  state->camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
  state->camera_speed = 0.05f;
  state->camera_fov = 90.0f;
  state->camera_near = 0.1f;
  state->camera_far = 100.0f;
  camera_update_matrix(state);

  state->mouse_has_moved = false;
  state->mouse_last_x = 0.0f;
  state->mouse_last_y = 0.0f;
  state->mouse_sensitivity = 0.1f;

  state->is_wireframe_on = false;

  return state;
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

  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  glfwSetWindowUserPointer(window, state);

  return window;
}

void init_objects(State *state) {
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

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(
    0, 3, GL_FLOAT, false, 8 * sizeof(real32), (void*)0
  );

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(
    1, 3, GL_FLOAT, false, 8 * sizeof(real32), (void*)(3 * sizeof(real32))
  );

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(
    2, 2, GL_FLOAT, false, 8 * sizeof(real32), (void*)(6 * sizeof(real32))
  );

  glBindVertexArray(0);

  /* glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); */

  uint32 alpaca_shader_program = shader_make_program_with_paths("src/alpaca.vert", "src/alpaca.frag");
  state->alpaca_shader_program = alpaca_shader_program;

  uint32 backpack_shader_program = shader_make_program_with_paths("src/backpack.vert", "src/backpack.frag");
  state->backpack_shader_program = backpack_shader_program;

  uint32 goose_shader_program = shader_make_program_with_paths("src/goose.vert", "src/goose.frag");
  state->goose_shader_program = goose_shader_program;

  state->vao = vao;

  int32 texture_width, texture_height, texture_n_channels;
  unsigned char *texture_data = util_load_image(
    "resources/alpaca.jpg", &texture_width, &texture_height, &texture_n_channels
  );
  if (texture_data) {
    uint32 test_texture;
    glGenTextures(1, &test_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, test_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    state->test_texture = test_texture;
  } else {
    log_error("Failed to load texture.");
  }
  /* stbi_image_free(texture_data); */

  state->n_models = 0;

#if 1
  Model *backpack_model = &state->models[state->n_models++];
  models_load_model(
    backpack_model, "resources/backpack/", "backpack.obj"
  );
#endif

  Model *goose_model = &state->models[state->n_models++];
  models_load_model(
    goose_model, "resources/", "miniGoose.fbx"
  );
}

void render(State *state) {
  real64 t = glfwGetTime();

  glClearColor(0.180f, 0.204f, 0.251f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, state->test_texture);

  glm::mat4 model = glm::mat4(1.0f);

  glm::mat4 view = glm::lookAt(
    state->camera_pos, state->camera_pos + state->camera_front, state->camera_up
  );

  glm::mat4 projection = glm::perspective(
    glm::radians(state->camera_fov),
    (real32)state->window_width / (real32)state->window_height,
    state->camera_near, state->camera_far
  );

  // Alpaca

  glUseProgram(state->alpaca_shader_program);

  glUniform1f(
    glGetUniformLocation(state->alpaca_shader_program, "t"),
    (real32)t
  );

  glUniformMatrix4fv(
    glGetUniformLocation(state->alpaca_shader_program, "view"),
    1, GL_FALSE, glm::value_ptr(view)
  );

  glUniformMatrix4fv(
    glGetUniformLocation(state->alpaca_shader_program, "projection"),
    1, GL_FALSE, glm::value_ptr(projection)
  );

  glBindVertexArray(state->vao);
  /* glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); */
  for (uint8 i = 0; i < 10; i++) {
    real32 model_angle = 20.0f * i;
    model = glm::translate(glm::mat4(1.0f), state->cube_positions[i]);
    model = glm::rotate(model, (real32)t * glm::radians(model_angle), glm::vec3(1.0f, 0.3f, 0.5f));
    glUniformMatrix4fv(
      glGetUniformLocation(state->alpaca_shader_program, "model"),
      1, GL_FALSE, glm::value_ptr(model)
    );

    glDrawArrays(GL_TRIANGLES, 0, 36);
  }
  glBindVertexArray(0);

  // Backpack

#if 1
  glUseProgram(state->backpack_shader_program);

  glUniformMatrix4fv(
    glGetUniformLocation(state->backpack_shader_program, "view"),
    1, GL_FALSE, glm::value_ptr(view)
  );

  glUniformMatrix4fv(
    glGetUniformLocation(state->backpack_shader_program, "projection"),
    1, GL_FALSE, glm::value_ptr(projection)
  );

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
  model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
  glUniformMatrix4fv(
    glGetUniformLocation(state->backpack_shader_program, "model"),
    1, GL_FALSE, glm::value_ptr(model)
  );
  models_draw_model(&state->models[0], state->backpack_shader_program);
#endif

  // Goose

  glUseProgram(state->goose_shader_program);

  glUniform1f(
    glGetUniformLocation(state->goose_shader_program, "t"),
    (real32)t
  );

  glUniformMatrix4fv(
    glGetUniformLocation(state->goose_shader_program, "view"),
    1, GL_FALSE, glm::value_ptr(view)
  );

  glUniformMatrix4fv(
    glGetUniformLocation(state->goose_shader_program, "projection"),
    1, GL_FALSE, glm::value_ptr(projection)
  );

  model = glm::mat4(1.0f);
  model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
  model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
  model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  glUniformMatrix4fv(
    glGetUniformLocation(state->goose_shader_program, "model"),
    1, GL_FALSE, glm::value_ptr(model)
  );
  models_draw_model(&state->models[1], state->goose_shader_program);
}

void main_loop(GLFWwindow *window, State *state) {
  while(!glfwWindowShouldClose(window)) {
    process_input_continuous(window, state);
    render(state);
    glfwSwapBuffers(window);
    glfwPollEvents();
  }
}

void destroy_window() {
  glfwTerminate();
}

int main() {
  State *state = init_state();
  GLFWwindow *window = init_window(state);
  if (!window) {
    return -1;
  }
  init_objects(state);
  main_loop(window, state);
  destroy_window();
  return 0;
}
