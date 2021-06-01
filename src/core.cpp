#include <thread>
#include "../src_external/pstr.h"
#include "types.hpp"
#include "logs.hpp"
#include "util.hpp"
#include "renderer.hpp"
#include "internals.hpp"
#include "engine.hpp"
#include "state.hpp"
#include "peony_parser.hpp"
#include "core.hpp"
#include "intrinsics.hpp"


namespace core {
  void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
    State *state = memory_and_state->state;
    MemoryPool *asset_memory_pool = memory_and_state->asset_memory_pool;
    logs::info(
      "Window is now: %d x %d", state->window_size.width, state->window_size.height
    );
    state->window_size.width = width;
    state->window_size.height = height;
    cameras::update_matrices(
      state->cameras_state.camera_active,
      state->window_size.width,
      state->window_size.height
    );
    cameras::update_ui_matrices(
      state->cameras_state.camera_active,
      state->window_size.width,
      state->window_size.height
    );
    gui::update_screen_dimensions(
      &state->gui_state, state->window_size.width, state->window_size.height
    );
    renderer::resize_renderer_buffers(
      asset_memory_pool,
      &state->materials_state.materials,
      &state->renderer_state.builtin_textures,
      width,
      height
    );
  }


  void mouse_button_callback(
    GLFWwindow *window, int button, int action, int mods
  ) {
    MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
    State *state = memory_and_state->state;

    input::update_mouse_button(&state->input_state, button, action, mods);
    gui::update_mouse_button(&state->gui_state);
  }


  void mouse_callback(GLFWwindow *window, real64 x, real64 y) {
    MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
    State *state = memory_and_state->state;

    v2 mouse_pos = v2(x, y);
    input::update_mouse(&state->input_state, mouse_pos);

    if (state->renderer_state.is_cursor_enabled) {
      gui::update_mouse(&state->gui_state);
    } else {
      cameras::update_mouse(
        state->cameras_state.camera_active,
        state->input_state.mouse_3d_offset
      );
    }
  }


  void key_callback(
    GLFWwindow* window,
    int key, int scancode, int action, int mods
  ) {
    MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
    State *state = memory_and_state->state;
    input::update_keys(&state->input_state, key, scancode, action, mods);
  }


  void char_callback(
    GLFWwindow* window, uint32 codepoint
  ) {
    MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
    State *state = memory_and_state->state;
    input::update_text_input(&state->input_state, codepoint);
  }


  GLFWwindow* init_window(WindowSize *window_size) {
    glfwInit();

    logs::info("Using OpenGL 4.1");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    #if defined(PLATFORM_MACOS)
      // macOS requires a forward compatible context
      // This means the highest OpenGL version will be used that is at least the version
      // we specified, and that contains no breaking changes from the version we specified
      glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    #endif

    #if USE_OPENGL_DEBUG
      logs::info("Using OpenGL debug context");
      glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    #endif

    // Remove window decorations (border etc.)
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    // For fullscreen windows, do not discard our video mode when minimised
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);

    // Create the window. Right now we're working with screencoord sizes,
    // not pixels!

    #if USE_FULLSCREEN
      int32 n_monitors;
      GLFWmonitor **monitors = glfwGetMonitors(&n_monitors);
      GLFWmonitor *target_monitor = monitors[TARGET_MONITOR];
      const GLFWvidmode *video_mode = glfwGetVideoMode(target_monitor);
      glfwWindowHint(GLFW_RED_BITS, video_mode->redBits);
      glfwWindowHint(GLFW_GREEN_BITS, video_mode->greenBits);
      glfwWindowHint(GLFW_BLUE_BITS, video_mode->blueBits);
      glfwWindowHint(GLFW_REFRESH_RATE, video_mode->refreshRate);

      window_size->screencoord_width = video_mode->width;
      window_size->screencoord_height = video_mode->height;

      GLFWwindow *window = glfwCreateWindow(
        window_size->screencoord_width, window_size->screencoord_height,
        WINDOW_TITLE,
        #if USE_WINDOWED_FULLSCREEN
          nullptr, nullptr
        #else
          target_monitor, nullptr
        #endif
      );
    #else
      window_size.screencoord_width = 1920;
      window_size.screencoord_height = 1080;

      GLFWwindow *window = glfwCreateWindow(
        window_size.screencoord_width, window_size.screencoord_height,
        WINDOW_TITLE,
        nullptr, nullptr
      );

      glfwSetWindowPos(window, 200, 200);
    #endif

    if (!window) {
      logs::fatal("Failed to create GLFW window");
      return nullptr;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
      logs::fatal("Failed to initialize GLAD");
      return nullptr;
    }

    if (!GLAD_GL_ARB_texture_cube_map_array) {
      logs::fatal("No support for GLAD_GL_ARB_texture_cube_map_array");
    }
    if (!GLAD_GL_ARB_texture_storage) {
      logs::fatal("No support for GLAD_GL_ARB_texture_storage");
    }
    if (!GLAD_GL_ARB_buffer_storage) {
      logs::warning("No support for GLAD_GL_ARB_buffer_storage");
    }

    // TODO: Remove GL_EXT_debug_marker from GLAD
    // TODO: Remove GL_EXT_debug_label from GLAD
    // TODO: Remove GL_ARB_texture_storage_multisample from GLAD

    #if USE_OPENGL_DEBUG
      if (GLAD_GL_AMD_debug_output || GLAD_GL_ARB_debug_output || GLAD_GL_KHR_debug) {
        GLint flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
          glEnable(GL_DEBUG_OUTPUT);
          glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
          glDebugMessageCallback(util::debug_message_callback, nullptr);
          glDebugMessageControl(
            GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE
          );
        } else {
          logs::fatal("Tried to initialise OpenGL debug output but couldn't");
        }
      } else {
        logs::warning(
          "Tried to initialise OpenGL debug output but none of "
          "[GL_AMD_debug_output, GL_ARB_debug_output, GL_KHR_debug] "
          "are supported on this system. Skipping."
        );
      }
    #endif

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glLineWidth(2.0f);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Get the framebuffer size. This is the actual window size in pixels.
    glfwGetFramebufferSize(window, &window_size->width, &window_size->height);
    glViewport(0, 0, window_size->width, window_size->height);

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCharCallback(window, char_callback);

    return window;
  }


  State* init_state(
    MemoryPool *state_memory_pool,
    MemoryPool *asset_memory_pool
  ) {
    State *state = MEMORY_PUSH(state_memory_pool, State, "state");
    state->window = init_window(&state->window_size);
    if (!state->window) { return nullptr; }

    engine::init(&state->engine_state, asset_memory_pool);
    materials::init(&state->materials_state, asset_memory_pool);

    // NOTE: Left off reviewing here
    renderer::init(
      &state->renderer_state,
      asset_memory_pool,
      state->window_size.width, state->window_size.height,
      state->window
    );
    internals::init(
      &state->engine_state,
      &state->renderer_state,
      &state->materials_state
    );
    gui::init(
      &state->gui_state,
      asset_memory_pool,
      &state->input_state,
      state->window_size.width, state->window_size.height
    );
    debugdraw::init(&state->debug_draw_state, asset_memory_pool);
    input::init(&state->input_state, state->window);
    lights::init(&state->lights_state);
    tasks::init(&state->tasks_state, asset_memory_pool);
    anim::init(&state->anim_state, asset_memory_pool);
    cameras::init(
      &state->cameras_state,
      state->window_size.width, state->window_size.height
    );

    return state;
  }


  void destroy_state(State *state) {
    glfwTerminate();
  }
}


int core::run() {
  // Create memory pools
  MemoryPool state_memory_pool = {.size = sizeof(State)};
  defer { memory::destroy_memory_pool(&state_memory_pool); };
  MemoryPool asset_memory_pool = {.size = util::mb_to_b(1024)};
  defer { memory::destroy_memory_pool(&asset_memory_pool); };

  // Make state
  State *state = init_state(&state_memory_pool, &asset_memory_pool);
  if (!state) { return EXIT_FAILURE; }
  defer { destroy_state(state); };

  // Set up globals
  MemoryAndState memory_and_state = {&asset_memory_pool, state};
  glfwSetWindowUserPointer(state->window, &memory_and_state);

  // Set up loading threads
  std::mutex loading_thread_mutex;
  std::thread loading_threads[N_LOADING_THREADS];
  range (0, N_LOADING_THREADS) {
    loading_threads[idx] = std::thread(
      tasks::run_loading_loop,
      &state->tasks_state,
      &loading_thread_mutex,
      &state->engine_state.should_stop,
      idx
    );
  }
  defer { range (0, N_LOADING_THREADS) { loading_threads[idx].join(); } };

  // Run main loop
  engine::run_main_loop(
    &state->engine_state,
    &state->renderer_state,
    &state->materials_state,
    &state->cameras_state,
    &state->gui_state,
    &state->input_state,
    &state->lights_state,
    &state->tasks_state,
    &state->anim_state,
    state->window,
    &state->window_size
  );

  return EXIT_SUCCESS;
}
