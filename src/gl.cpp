#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4201)

#include <cstdlib>
#include <cstdarg>
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

#include "gl.h"


void log_error(const char* format, ...) {
  va_list vargs;
  fprintf(stderr, "error | ");
  va_start(vargs, format);
  vfprintf(stderr, format, vargs);
  fprintf(stderr, "\n");
  va_end(vargs);
}

void log_info(const char* format, ...) {
  va_list vargs;
  fprintf(stdout, "info  | ");
  va_start(vargs, format);
  vfprintf(stdout, format, vargs);
  fprintf(stdout, "\n");
  va_end(vargs);
}

void log_newline() {
  fprintf(stdout, "\n");
}

char* load_file(const char* path) {
  FILE* f = fopen(path, "rb");
  if (!f) {
    log_error("Could not open file %s.", path);
    return nullptr;
  }

  fseek(f, 0, SEEK_END);
  size_t fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  char* string = (char*)malloc(fsize + 1);
  size_t result = fread(string, fsize, 1, f);
  fclose(f);
  if (result != 1) {
    log_error("Could not read from file %s.", path);
    return nullptr;
  }

  string[fsize] = 0;

  return string;
}

void assert_shader_status_ok(uint32 shader) {
  int32 status;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

  char message[512];
  glGetShaderInfoLog(shader, 512, NULL, message);

  log_info("[assertShaderStatusOk] Compilation for shader %d", shader);
  log_info("Status: %d", status);
  log_info("Message: %s", message);

  if (status == false) {
    log_error("[assertShaderStatusOk] Shader compilation failed");
    exit(EXIT_FAILURE);
  }

  log_newline();
}

void assert_program_status_ok(uint32 shader) {
  int32 status;
  glGetProgramiv(shader, GL_COMPILE_STATUS, &status);

  char message[512];
  glGetProgramInfoLog(shader, 512, NULL, message);

  log_info("[assertProgramStatusOk] Loading program");
  log_info("Status: %d", status);
  log_info("Message: %s", message);

  if (status == false) {
    log_error("[assertProgramStatusOk] Program loading failed");
    exit(EXIT_FAILURE);
  }

  log_newline();
}

uint32 load_shader(const char* source, GLenum shaderType) {
  uint32 shader = glCreateShader(shaderType);
  glShaderSource(shader, 1, &source, NULL);
  glCompileShader(shader);
  assert_shader_status_ok(shader);
  return shader;
}

uint32 make_shader_program(uint32 vertexShader, uint32 fragmentShader) {
  uint32 shader_program = glCreateProgram();
  glAttachShader(shader_program, vertexShader);
  glAttachShader(shader_program, fragmentShader);
  glLinkProgram(shader_program);
  assert_program_status_ok(shader_program);
  return shader_program;
}

uint32 make_shader_program_with_paths(const char* vertPath, const char* fragPath) {
  return make_shader_program(
    load_shader(load_file(vertPath), GL_VERTEX_SHADER),
    load_shader(load_file(fragPath), GL_FRAGMENT_SHADER)
  );
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  State *state = (State*)glfwGetWindowUserPointer(window);
  state->window_width = width;
  state->window_height = height;
  log_info("%d x %d", state->window_width, state->window_height);
  glViewport(0, 0, width, height);
}

void process_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, true);
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

  uint32 shader_program = make_shader_program_with_paths("src/test.vert", "src/test.frag");

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

  /* glm::mat4 model = glm::mat4(1.0f); */
  glm::mat4 view = glm::mat4(1.0f);
  glm::mat4 projection;

  /* model = glm::rotate(model, (float)t * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f)); */
  view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
  projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

  /* glUniformMatrix4fv( */
  /*   glGetUniformLocation(state->shader_program, "model"), */
  /*   1, GL_FALSE, glm::value_ptr(model) */
  /* ); */

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
    /* state->shader_program.setMat4("model", model); */
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
