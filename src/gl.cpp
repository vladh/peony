#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4201)
#pragma warning(disable : 4127)

#include <cstdlib>
#include <cstring>
#include <cmath>
#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "gl.hpp"
#include "log.hpp"
#include "util.hpp"
#include "shader.hpp"
#include "camera.hpp"


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

void process_input(GLFWwindow *window) {
  State *state = (State*)glfwGetWindowUserPointer(window);

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
  }

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
    camera_move_front_back(state, 1);
  }

  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
    camera_move_front_back(state, -1);
  }

  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
    camera_move_left_right(state, -1);
  }

  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
    camera_move_left_right(state, 1);
  }

  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
    camera_move_up_down(state, 1);
  }

  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
    camera_move_up_down(state, -1);
  }
}

State* init_state() {
  State *state = (State*)malloc(sizeof(State));
  state->window_width = 800;
  state->window_height = 600;
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

  glVertexAttribPointer(0, 3, GL_FLOAT, false, 8 * sizeof(real32), (void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 3, GL_FLOAT, false, 8 * sizeof(real32), (void*)(3 * sizeof(real32)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 2, GL_FLOAT, false, 8 * sizeof(real32), (void*)(6 * sizeof(real32)));
  glEnableVertexAttribArray(2);

  glBindVertexArray(0);

  /* glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); */

  uint32 shader_program = shader_make_program_with_paths("src/test.vert", "src/test.frag");

  state->shader_program = shader_program;
  state->vao = vao;

  int32 texture_width, texture_height, texture_n_channels;
  stbi_set_flip_vertically_on_load(true);
  unsigned char *texture_data = stbi_load(
    "resources/alpaca.jpg", &texture_width, &texture_height, &texture_n_channels, 0
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
  stbi_image_free(texture_data);
}

void render(State *state) {
  real64 t = glfwGetTime();

  /* glClearColor(0.0f, 0.333f, 0.933f, 1.0f); */
  glClearColor(0.0f, 0.000f, 1.000f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  uint32 t_uniform_location = glGetUniformLocation(state->shader_program, "t");
  glUseProgram(state->shader_program);
  glUniform1f(t_uniform_location, (real32)t);

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, state->test_texture);

  glm::mat4 view = glm::lookAt(
    state->camera_pos, state->camera_pos + state->camera_front, state->camera_up
  );

  glm::mat4 projection = glm::perspective(
    glm::radians(state->camera_fov),
    (real32)state->window_width / (real32)state->window_height,
    state->camera_near, state->camera_far
  );

  glUniformMatrix4fv(
    glGetUniformLocation(state->shader_program, "view"),
    1, GL_FALSE, glm::value_ptr(view)
  );

  glUniformMatrix4fv(
    glGetUniformLocation(state->shader_program, "projection"),
    1, GL_FALSE, glm::value_ptr(projection)
  );

  glBindVertexArray(state->vao);
  /* glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0); */
  for (uint8 i = 0; i < 10; i++) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, state->cube_positions[i]);
    float angle = 20.0f * i;
    model = glm::rotate(model, (float)t * glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
    glUniformMatrix4fv(
      glGetUniformLocation(state->shader_program, "model"),
      1, GL_FALSE, glm::value_ptr(model)
    );

    glDrawArrays(GL_TRIANGLES, 0, 36);
  }
  glBindVertexArray(0);
}

void main_loop(GLFWwindow *window, State *state) {
  while(!glfwWindowShouldClose(window)) {
    process_input(window);
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
