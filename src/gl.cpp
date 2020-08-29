#define _CRT_SECURE_NO_WARNINGS

#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <stdio.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

  printf("\n");
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

  printf("\n");
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
  log_info("%d\n", state->window_width);
  state->window_width = width;
  state->window_height = height;
  log_info("%d\n", state->window_width);
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
     0.5f,  0.5f,  0.0f,  1.0f, 0.0f, 0.0f,  1.0f, 1.0f, // top right
     0.5f, -0.5f,  0.0f,  0.0f, 1.0f, 0.0f,  1.0f, 0.0f, // bottom right
    -0.5f, -0.5f,  0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // bottom left
    -0.5f,  0.5f,  0.0f,  1.0f, 0.0f, 1.0f,  0.0f, 1.0f, // top left
  };
  uint32 test_indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3 // second triangle
  };
  memcpy(state->test_vertices, test_vertices, sizeof(test_vertices));
  memcpy(state->test_indices, test_indices, sizeof(test_indices));
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

#if 0
  int32 texture_width, texture_height, texture_n_channels;
  unsigned char *texture_data = stbi_load(
    "resources/container.jpg", &texture_width, &texture_height, &texture_n_channels, 0
  );
  if (texture_data) {
    uint32 test_texture;
    glGenTextures(1, &test_texture);
    glBindTexture(GL_TEXTURE_2D, test_texture);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGB, texture_width, texture_height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture_data
    );
    glGenerateMipmap(GL_TEXTURE_2D);
    state->test_texture = test_texture;
  } else {
    log_error("Failed to load texture.");
  }
  stbi_image_free(texture_data);
#endif
}

void render(State *state) {
  glClearColor(0.0f, 0.333f, 0.933f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(state->shader_program);
#if 0
  glBindTexture(GL_TEXTURE_2D, state->test_texture);
#endif
  glBindVertexArray(state->vao);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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
