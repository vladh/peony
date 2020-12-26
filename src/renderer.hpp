#ifndef OPENGL_HPP
#define OPENGL_HPP

constexpr bool32 USE_OPENGL_DEBUG = false;

struct WindowInfo;

namespace Renderer {
  void resize_renderer_buffers(Memory *memory, State *state);
  void init_ubo(Memory *memory, State *state);
  void init_shadowmaps(Memory *memory, State *state);
  void init_g_buffer(Memory *memory, State *state);
  void init_l_buffer(Memory *memory, State *state);
  void init_blur_buffers(Memory *memory, State *state);
  void update_drawing_options(State *state, GLFWwindow *window);
  void copy_scene_data_to_ubo(
    Memory *memory, State *state,
    uint32 current_shadow_light_idx,
    uint32 current_shadow_light_type,
    bool32 is_blur_horizontal
  );
  void copy_scene_data_to_ubo(Memory *memory, State *state);
  void init_window(WindowInfo *window_info);
  void framebuffer_size_callback(GLFWwindow* window, int width, int height);
  void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
  void mouse_callback(GLFWwindow *window, real64 x, real64 y);
  void key_callback(
    GLFWwindow* window, int key, int scancode, int action, int mods
  );
}

#endif
