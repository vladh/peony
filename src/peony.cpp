#define _CRT_SECURE_NO_WARNINGS

#define USE_OPENGL_DEBUG false
#define USE_TIMERS true
#define USE_VLD false
#define USE_MEMORY_DEBUG_LOGS false
#define USE_MEMORYPOOL_ITEM_DEBUG false
#define USE_CACHELINE_SIZE_DISPLAY false
#define USE_FULLSCREEN false

#include "peony.hpp"

global_variable uint32 global_oopses = 0;

#include "log.cpp"
#include "pack.cpp"
#include "render.cpp"
#include "util.cpp"
#include "task.cpp"
#include "peony_file_parser.cpp"
#include "texture_name_pool.cpp"
#include "texture.cpp"
#include "font_asset.cpp"
#include "shader_asset.cpp"
#include "persistent_pbo.cpp"
#include "material.cpp"
#include "texture_atlas.cpp"
#include "camera.cpp"
#include "memory_pool.cpp"
#include "memory.cpp"
#include "input_manager.cpp"
#include "entity_manager.cpp"
#include "behavior_component.cpp"
#include "drawable_component.cpp"
#include "light_component.cpp"
#include "spatial_component.cpp"
#include "behavior_component_manager.cpp"
#include "drawable_component_manager.cpp"
#include "light_component_manager.cpp"
#include "spatial_component_manager.cpp"
#include "gui_manager.cpp"
#include "model_asset.cpp"
#include "world.cpp"
#include "state.cpp"


void init_ubo(Memory *memory, State *state) {
  glGenBuffers(1, &state->ubo_shader_common);
  glBindBuffer(GL_UNIFORM_BUFFER, state->ubo_shader_common);
  glBufferData(GL_UNIFORM_BUFFER, sizeof(ShaderCommon), NULL, GL_STATIC_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  glBindBufferRange(GL_UNIFORM_BUFFER, 0, state->ubo_shader_common, 0, sizeof(ShaderCommon));
}


void init_shadowmaps(Memory *memory, State *state) {
  // Cube
  glGenFramebuffers(1, &state->cube_shadowmaps_framebuffer);
  glGenTextures(1, &state->cube_shadowmaps);
  glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, state->cube_shadowmaps);

  glTexStorage3D(
    GL_TEXTURE_CUBE_MAP_ARRAY, 1, GL_DEPTH_COMPONENT32F,
    state->cube_shadowmap_width, state->cube_shadowmap_height,
    6 * MAX_N_LIGHTS
  );

  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glBindFramebuffer(GL_FRAMEBUFFER, state->cube_shadowmaps_framebuffer);
  glFramebufferTexture(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, state->cube_shadowmaps, 0
  );

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    log_fatal("Framebuffer not complete!");
  }

  state->cube_shadowmaps_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "cube_shadowmaps_texture")
  ) Texture(
    GL_TEXTURE_CUBE_MAP_ARRAY,
    TextureType::shadowmap, state->cube_shadowmaps,
    state->cube_shadowmap_width, state->cube_shadowmap_height, 1
  );

  // Texture
  glGenFramebuffers(1, &state->texture_shadowmaps_framebuffer);
  glGenTextures(1, &state->texture_shadowmaps);
  glBindTexture(GL_TEXTURE_2D_ARRAY, state->texture_shadowmaps);

  glTexStorage3D(
    GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT32F,
    state->texture_shadowmap_width, state->texture_shadowmap_height,
    MAX_N_LIGHTS
  );

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  real32 border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
  glTexParameterfv(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BORDER_COLOR, border_color);
  glBindFramebuffer(GL_FRAMEBUFFER, state->texture_shadowmaps_framebuffer);
  glFramebufferTexture(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, state->texture_shadowmaps, 0
  );

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    log_fatal("Framebuffer not complete!");
  }

  state->texture_shadowmaps_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "texture_shadowmaps_texture")
  ) Texture(
    GL_TEXTURE_2D_ARRAY,
    TextureType::shadowmap, state->texture_shadowmaps,
    state->texture_shadowmap_width, state->texture_shadowmap_height, 1
  );
}


void init_g_buffer(Memory *memory, State *state) {
  glGenFramebuffers(1, &state->g_buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, state->g_buffer);

  uint32 g_position_texture_name;
  uint32 g_normal_texture_name;
  uint32 g_albedo_texture_name;
  uint32 g_pbr_texture_name;

  glGenTextures(1, &g_position_texture_name);
  glGenTextures(1, &g_normal_texture_name);
  glGenTextures(1, &g_albedo_texture_name);
  glGenTextures(1, &g_pbr_texture_name);

  state->g_position_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "g_position_texture")
  ) Texture(
    GL_TEXTURE_2D, TextureType::g_position, g_position_texture_name,
    state->window_info.width, state->window_info.height, 4
  );
  state->g_normal_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "g_normal_texture")
  ) Texture(
    GL_TEXTURE_2D, TextureType::g_normal, g_normal_texture_name,
    state->window_info.width, state->window_info.height, 4
  );
  state->g_albedo_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "g_albedo_texture")
  ) Texture(
    GL_TEXTURE_2D, TextureType::g_albedo, g_albedo_texture_name,
    state->window_info.width, state->window_info.height, 4
  );
  state->g_pbr_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "g_pbr_texture")
  ) Texture(
    GL_TEXTURE_2D, TextureType::g_pbr, g_pbr_texture_name,
    state->window_info.width, state->window_info.height, 4
  );

  glBindTexture(GL_TEXTURE_2D, state->g_position_texture->texture_name);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA16F,
    state->g_position_texture->width, state->g_position_texture->height,
    0, GL_RGBA, GL_FLOAT, NULL
  );
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
    state->g_position_texture->texture_name, 0
  );

  glBindTexture(GL_TEXTURE_2D, state->g_normal_texture->texture_name);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA16F,
    state->g_normal_texture->width, state->g_normal_texture->height,
    0, GL_RGBA, GL_FLOAT, NULL
  );
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
    state->g_normal_texture->texture_name, 0
  );

  glBindTexture(GL_TEXTURE_2D, state->g_albedo_texture->texture_name);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA,
    state->g_albedo_texture->width, state->g_albedo_texture->height,
    0, GL_RGBA, GL_UNSIGNED_BYTE, NULL
  );
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D,
    state->g_albedo_texture->texture_name, 0
  );

  glBindTexture(GL_TEXTURE_2D, state->g_pbr_texture->texture_name);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA,
    state->g_pbr_texture->width, state->g_pbr_texture->height,
    0, GL_RGBA, GL_UNSIGNED_BYTE, NULL
  );
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT3, GL_TEXTURE_2D,
    state->g_pbr_texture->texture_name, 0
  );

  uint32 attachments[4] = {
    GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1,
    GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3
  };
  glDrawBuffers(4, attachments);

  uint32 rbo_depth;
  glGenRenderbuffers(1, &rbo_depth);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
  glRenderbufferStorage(
    GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
    state->window_info.width, state->window_info.height
  );
  glFramebufferRenderbuffer(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth
  );

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    log_fatal("Framebuffer not complete!");
  }
}


void init_l_buffer(Memory *memory, State *state) {
  glGenFramebuffers(1, &state->l_buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, state->l_buffer);

  uint32 l_color_texture_name;
  glGenTextures(1, &l_color_texture_name);
  state->l_color_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "l_color_texture")
  ) Texture(
    GL_TEXTURE_2D, TextureType::l_color, l_color_texture_name,
    state->window_info.width, state->window_info.height, 4
  );
  glBindTexture(GL_TEXTURE_2D, state->l_color_texture->texture_name);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA16F,
    state->l_color_texture->width, state->l_color_texture->height,
    0, GL_RGBA, GL_FLOAT, NULL
  );
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
    state->l_color_texture->texture_name, 0
  );

  uint32 l_bright_color_texture_name;
  glGenTextures(1, &l_bright_color_texture_name);
  state->l_bright_color_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "l_bright_color_texture")
  ) Texture(
    GL_TEXTURE_2D, TextureType::l_bright_color, l_bright_color_texture_name,
    state->window_info.width, state->window_info.height, 4
  );
  glBindTexture(GL_TEXTURE_2D, state->l_bright_color_texture->texture_name);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA16F,
    state->l_bright_color_texture->width, state->l_bright_color_texture->height,
    0, GL_RGBA, GL_FLOAT, NULL
  );
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D,
    state->l_bright_color_texture->texture_name, 0
  );

  uint32 attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachments);

#if 0
  uint32 rbo_depth;
  glGenRenderbuffers(1, &rbo_depth);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_depth);
  glRenderbufferStorage(
    GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
    state->window_info.width, state->window_info.height
  );
  glFramebufferRenderbuffer(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo_depth
  );
#else
  uint32 l_depth_texture_name;
  glGenTextures(1, &l_depth_texture_name);
  state->l_depth_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "l_depth_texture")
  ) Texture(
    GL_TEXTURE_2D, TextureType::l_depth, l_depth_texture_name,
    state->window_info.width, state->window_info.height, 1
  );
  glBindTexture(GL_TEXTURE_2D, state->l_depth_texture->texture_name);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
    state->l_depth_texture->width, state->l_depth_texture->height,
    0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL
  );
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
    state->l_depth_texture->texture_name, 0
  );
#endif

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    log_fatal("Framebuffer not complete!");
  }
}


void init_blur_buffers(Memory *memory, State *state) {
  glGenFramebuffers(1, &state->blur1_buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, state->blur1_buffer);
  uint32 blur1_texture_name;
  glGenTextures(1, &blur1_texture_name);
  state->blur1_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "blur1_texture")
  ) Texture(
    GL_TEXTURE_2D, TextureType::blur1, blur1_texture_name,
    state->window_info.width, state->window_info.height, 4
  );
  glBindTexture(GL_TEXTURE_2D, state->blur1_texture->texture_name);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA16F,
    state->blur1_texture->width, state->blur1_texture->height,
    0, GL_RGBA, GL_FLOAT, NULL
  );
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
    state->blur1_texture->texture_name, 0
  );

  glGenFramebuffers(1, &state->blur2_buffer);
  glBindFramebuffer(GL_FRAMEBUFFER, state->blur2_buffer);
  uint32 blur2_texture_name;
  glGenTextures(1, &blur2_texture_name);
  state->blur2_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "blur2_texture")
  ) Texture(
    GL_TEXTURE_2D, TextureType::blur2, blur2_texture_name,
    state->window_info.width, state->window_info.height, 4
  );
  glBindTexture(GL_TEXTURE_2D, state->blur2_texture->texture_name);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(
    GL_TEXTURE_2D, 0, GL_RGBA16F,
    state->blur2_texture->width, state->blur2_texture->height,
    0, GL_RGBA, GL_FLOAT, NULL
  );
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
    state->blur2_texture->texture_name, 0
  );

  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    log_fatal("Framebuffer not complete!");
  }
}


void reload_shaders(Memory *memory, State *state) {
  for (uint32 idx = 0; idx < state->shader_assets.size; idx++) {
    ShaderAsset *shader_asset = state->shader_assets[idx];
    shader_asset->load(memory);
  }
}


void update_drawing_options(State *state, GLFWwindow *window) {
  if (state->is_cursor_disabled) {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  } else {
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  }

  if (state->should_use_wireframe) {
    // This will be handled in the rendering loop.
  } else {
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }
}


void update_light_position(State *state, real32 amount) {
  for (uint32 idx = 0; idx < state->light_component_manager.components->size; idx++) {
    LightComponent *light_component = state->light_component_manager.components->get(idx);
    if (light_component -> type == LightType::directional) {
      state->dir_light_angle += amount;
      light_component->direction = glm::vec3(
        sin(state->dir_light_angle), -cos(state->dir_light_angle), 0.0f
      );
      break;
    }
  }
}


void process_input(GLFWwindow *window, State *state, Memory *memory) {
  // Continuous
  if (state->input_manager.is_key_down(GLFW_KEY_W)) {
    state->camera_active->move_front_back(1, state->dt);
  }

  if (state->input_manager.is_key_down(GLFW_KEY_S)) {
    state->camera_active->move_front_back(-1, state->dt);
  }

  if (state->input_manager.is_key_down(GLFW_KEY_A)) {
    state->camera_active->move_left_right(-1, state->dt);
  }

  if (state->input_manager.is_key_down(GLFW_KEY_D)) {
    state->camera_active->move_left_right(1, state->dt);
  }

  if (state->input_manager.is_key_down(GLFW_KEY_Z)) {
    update_light_position(state, 0.10f * (real32)state->dt);
  }

  if (state->input_manager.is_key_down(GLFW_KEY_X)) {
    update_light_position(state, -0.10f * (real32)state->dt);
  }

  if (state->input_manager.is_key_down(GLFW_KEY_SPACE)) {
    state->camera_active->move_up_down(1, state->dt);
  }

  if (state->input_manager.is_key_down(GLFW_KEY_LEFT_CONTROL)) {
    state->camera_active->move_up_down(-1, state->dt);
  }

  // Transient
  if (state->input_manager.is_key_now_down(GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(window, true);
  }

  if (state->input_manager.is_key_now_down(GLFW_KEY_C)) {
    state->is_cursor_disabled = !state->is_cursor_disabled;
    update_drawing_options(state, window);
  }

  if (state->input_manager.is_key_now_down(GLFW_KEY_TAB)) {
    state->should_pause = !state->should_pause;
  }

  if (state->input_manager.is_key_now_down(GLFW_KEY_BACKSPACE)) {
    state->should_hide_ui = !state->should_hide_ui;
  }

  if (state->input_manager.is_key_down(GLFW_KEY_ENTER)) {
    state->should_manually_advance_to_next_frame = true;
  }
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
  State *state = memory_and_state->state;
  Memory *memory = memory_and_state->memory;
  log_info(
    "Window is now: %d x %d", state->window_info.width, state->window_info.height
  );
  state->window_info.width = width;
  state->window_info.height = height;
  state->camera_active->update_matrices(
    state->window_info.width, state->window_info.height
  );
  state->camera_active->update_ui_matrices(
    state->window_info.width, state->window_info.height
  );
  state->gui_manager.update_screen_dimensions(
    state->window_info.width, state->window_info.height
  );

  // TODO: Only regenerate once we're done resizing, not for every little bit of the resize.
  init_g_buffer(memory, state);
  init_l_buffer(memory, state);
  init_blur_buffers(memory, state);

  for (uint32 idx = 0; idx < state->model_assets.size; idx++) {
    ModelAsset *model_asset = state->model_assets[idx];
    for (
      uint32 idx_mesh = 0; idx_mesh < model_asset->meshes.size; idx_mesh++
    ) {
      Mesh *mesh = model_asset->meshes[idx_mesh];
      if (
        mesh->material->textures.size > 0 &&
        mesh->material->is_screensize_dependent
      ) {
        Material *material = mesh->material;
        log_info("Found G-buffer dependent mesh in model %s", model_asset->name);
        for(
          uint32 idx_texture = 0; idx_texture < material->textures.size; idx_texture++
        ) {
          Texture *texture = material->textures[idx_texture];
          if (texture->type == TextureType::g_position) {
            material->textures.set(idx_texture, state->g_position_texture);
          } else if (texture->type == TextureType::g_normal) {
            material->textures.set(idx_texture, state->g_normal_texture);
          } else if (texture->type == TextureType::g_albedo) {
            material->textures.set(idx_texture, state->g_albedo_texture);
          } else if (texture->type == TextureType::g_pbr) {
            material->textures.set(idx_texture, state->g_pbr_texture);
          } else if (texture->type == TextureType::l_color) {
            material->textures.set(idx_texture, state->l_color_texture);
          } else if (texture->type == TextureType::l_bright_color) {
            material->textures.set(idx_texture, state->l_bright_color_texture);
          } else if (texture->type == TextureType::l_depth) {
            material->textures.set(idx_texture, state->l_depth_texture);
          } else if (texture->type == TextureType::blur1) {
            material->textures.set(idx_texture, state->blur1_texture);
          } else if (texture->type == TextureType::blur2) {
            material->textures.set(idx_texture, state->blur2_texture);
          }
        }
        model_asset->bind_texture_uniforms_for_mesh(mesh);
      }
    }
  }
}


void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
  MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
  State *state = memory_and_state->state;

  state->input_manager.update_mouse_button(button, action, mods);
  state->gui_manager.update_mouse_button();
}


void mouse_callback(GLFWwindow *window, real64 x, real64 y) {
  MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
  State *state = memory_and_state->state;

  glm::vec2 mouse_pos = glm::vec2(x, y);
  state->input_manager.update_mouse(mouse_pos);

  if (state->is_cursor_disabled) {
    state->camera_active->update_mouse(state->input_manager.mouse_3d_offset);
  } else {
    state->gui_manager.update_mouse();
  }
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
  State *state = memory_and_state->state;
  state->input_manager.update_keys(key, scancode, action, mods);
}


void init_window(WindowInfo *window_info) {
  strcpy(window_info->title, "hi lol");

  glfwInit();

  log_info("Using OpenGL 4.1");
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

#if defined(__APPLE__)
  log_info("Using GLFW_OPENGL_FORWARD_COMPAT");
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

#if USE_OPENGL_DEBUG
  log_info("Using OpenGL debug context");
  glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

  int32 n_monitors;
  GLFWmonitor **monitors = glfwGetMonitors(&n_monitors);
  GLFWmonitor *target_monitor = monitors[0];

  const GLFWvidmode *video_mode = glfwGetVideoMode(target_monitor);
  glfwWindowHint(GLFW_RED_BITS, video_mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, video_mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, video_mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, video_mode->refreshRate);
#if USE_FULLSCREEN
  window_info->width = video_mode->width;
  window_info->height = video_mode->height;
#else
  window_info->width = 1920;
  window_info->height = 1080;
#endif

  GLFWwindow *window = glfwCreateWindow(
    window_info->width, window_info->height, window_info->title,
    /* target_monitor, nullptr */
    nullptr, nullptr
  );
  if (!window) {
    log_fatal("Failed to create GLFW window");
    return;
  }
  window_info->window = window;
#if USE_FULLSCREEN
  glfwSetWindowPos(window, 0, 0);
#else
  glfwSetWindowPos(window, 200, 200);
#endif

  glfwMakeContextCurrent(window);
  glfwSwapInterval(0);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    log_fatal("Failed to initialize GLAD");
    return;
  }

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_MULTISAMPLE);

#if USE_OPENGL_DEBUG
  GLint flags;
  glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

  if (flags & GL_CONTEXT_FLAG_DEBUG_BIT) {
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    glDebugMessageCallback(Util::debug_message_callback, nullptr);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
  } else {
    log_fatal("Tried to initialise OpenGL debug output but couldn't");
  }
#endif

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glViewport(0, 0, window_info->width, window_info->height);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetMouseButtonCallback(window, mouse_button_callback);
  glfwSetKeyCallback(window, key_callback);
}


void copy_scene_data_to_ubo(
  Memory *memory, State *state,
  uint32 current_shadow_light_idx,
  uint32 current_shadow_light_type,
  bool32 is_blur_horizontal
) {
  ShaderCommon *shader_common = &state->shader_common;

  shader_common->view = state->camera_active->view;
  shader_common->projection = state->camera_active->projection;
  shader_common->ui_projection = state->camera_active->ui_projection;
  memcpy(
    shader_common->cube_shadowmap_transforms,
    state->cube_shadowmap_transforms,
    sizeof(state->cube_shadowmap_transforms)
  );
  memcpy(
    shader_common->texture_shadowmap_transforms,
    state->texture_shadowmap_transforms,
    sizeof(state->texture_shadowmap_transforms)
  );

  shader_common->camera_position = glm::vec4(state->camera_active->position, 1.0f);
  shader_common->camera_pitch = (float)state->camera_active->pitch;

  shader_common->camera_horizontal_fov = state->camera_active->horizontal_fov;
  shader_common->camera_vertical_fov = state->camera_active->vertical_fov;
  shader_common->camera_near_clip_dist = state->camera_active->near_clip_dist;
  shader_common->camera_far_clip_dist = state->camera_active->far_clip_dist;

  shader_common->current_shadow_light_idx = current_shadow_light_idx;
  shader_common->current_shadow_light_type = current_shadow_light_type;

  shader_common->shadow_far_clip_dist = state->shadowmap_far_clip_dist;
  shader_common->is_blur_horizontal = is_blur_horizontal;

  shader_common->exposure = state->camera_active->exposure;
  shader_common->t = (float)state->t;
  shader_common->window_width = state->window_info.width;
  shader_common->window_height = state->window_info.height;

  uint32 n_point_lights = 0;
  uint32 n_directional_lights = 0;

  for (uint32 idx = 0; idx < state->light_component_manager.components->size; idx++) {
    LightComponent *light_component =
      state->light_component_manager.components->get(idx);
    SpatialComponent *spatial_component = state->spatial_component_manager.get(
      light_component->entity_handle
    );

    if (!(
      light_component->is_valid() &&
      spatial_component->is_valid()
    )) {
      continue;
    }

    if (light_component->type == LightType::point) {
      shader_common->point_light_position[n_point_lights] = glm::vec4(
        spatial_component->position, 1.0f
      );
      shader_common->point_light_color[n_point_lights] =
        light_component->color;
      shader_common->point_light_attenuation[n_point_lights] =
        light_component->attenuation;
      n_point_lights++;
    } else if (light_component->type == LightType::directional) {
      shader_common->directional_light_position[n_directional_lights] =
        glm::vec4(spatial_component->position, 1.0f);
      shader_common->directional_light_direction[n_directional_lights] =
        glm::vec4(light_component->direction, 1.0f);
      shader_common->directional_light_color[n_directional_lights] =
        light_component->color;
      shader_common->directional_light_attenuation[n_directional_lights] =
        light_component->attenuation;
      n_directional_lights++;
    }
  }

  shader_common->n_point_lights = n_point_lights;
  shader_common->n_directional_lights = n_directional_lights;

  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderCommon), shader_common);
}


void copy_scene_data_to_ubo(Memory *memory, State *state) {
  copy_scene_data_to_ubo(memory, state, 0, 0, false);
}


void render_scene(
  Memory *memory,
  State *state,
  RenderPass::Flag render_pass,
  RenderMode render_mode
) {
  state->drawable_component_manager.draw_all(
    &state->spatial_component_manager,
    render_pass,
    render_mode,
    state->standard_depth_shader_asset
  );
}


void set_heading(
  State *state,
  const char *text, real32 opacity,
  real32 fadeout_duration, real32 fadeout_delay
) {
  state->heading_text = text;
  state->heading_opacity = opacity;
  state->heading_fadeout_duration = fadeout_duration;
  state->heading_fadeout_delay = fadeout_delay;
}


void get_entity_text_representation(
  char *text, State *state, Entity *entity, uint8 depth
) {
  EntityHandle handle = entity->handle;
  SpatialComponent *spatial_component = state->spatial_component_manager.get(handle);

  // Children will be drawn under their parents.
  if (
    depth == 0 &&
    spatial_component->is_valid() &&
    spatial_component->parent_entity_handle != Entity::no_entity_handle
  ) {
    return;
  }

  bool32 has_spatial_component = spatial_component->is_valid();
  bool32 has_drawable_component =
    state->drawable_component_manager.get(handle)->is_valid();
  bool32 has_light_component =
    state->light_component_manager.get(handle)->is_valid();
  bool32 has_behavior_component =
    state->behavior_component_manager.get(handle)->is_valid();

  for (uint8 level = 0; level < depth; level++) {
    strcat(text, "  ");
  }

  strcat(text, "- ");
  strcat(text, entity->debug_name);

  strcat(text, "@");
  // Because NUM_TO_STR only has 2048 entries
  assert(entity->handle < 2048);
  strcat(text, NUM_TO_STR[entity->handle]);

  if (
    !has_spatial_component &&
    !has_drawable_component &&
    !has_light_component &&
    !has_behavior_component
  ) {
    strcat(text, " (orphan)");
  }

  if (has_spatial_component) {
    strcat(text, " +S");
  }
  if (has_drawable_component) {
    strcat(text, " +D");
  }
  if (has_light_component) {
    strcat(text, " +L");
  }
  if (has_behavior_component) {
    strcat(text, " +B");
  }

  if (spatial_component->is_valid()) {
    // NOTE: This is super slow lol.
    uint32 n_children_found = 0;
    for (
      uint32 child_idx = 1;
      child_idx < state->spatial_component_manager.components->size;
      child_idx++
    ) {
      SpatialComponent *child_spatial_component =
        state->spatial_component_manager.components->get(child_idx);
      if (
        child_spatial_component->parent_entity_handle ==
          spatial_component->entity_handle
      ) {
        n_children_found++;
        if (n_children_found > 5) {
          continue;
        }
        EntityHandle child_handle = child_spatial_component->entity_handle;
        Entity *child_entity = state->entities.get(child_handle);

        if (text[strlen(text) - 1] != '\n') {
          strcat(text, "\n");
        }
        get_entity_text_representation(text, state, child_entity, depth + 1);
      }
    }
    if (n_children_found > 5) {
      for (uint8 level = 0; level < (depth + 1); level++) {
        strcat(text, "  ");
      }
      strcat(text, "(and ");
      strcat(text, NUM_TO_STR[n_children_found - 5]);
      strcat(text, " more)");
    }
  }

  if (text[strlen(text) - 1] != '\n') {
    strcat(text, "\n");
  }
}


void get_scene_text_representation(char *text, State *state) {
  strcpy(text, "");

  for (uint32 idx = 1; idx < state->entities.size; idx++) {
    Entity *entity = state->entities[idx];
    get_entity_text_representation(text, state, entity, 0);
  }

  if (text[strlen(text) - 1] == '\n') {
    text[strlen(text) - 1] = 0;
  }
}


void render_scene_ui(
  Memory *memory, State *state
){
  char debug_text[1 << 14];

  state->gui_manager.start_drawing();

  if (state->heading_opacity > 0.0f) {
    state->gui_manager.draw_heading(
      state->heading_text,
      glm::vec4(0.0f, 0.33f, 0.93f, state->heading_opacity)
    );
    if (state->heading_fadeout_delay > 0.0f) {
      state->heading_fadeout_delay -= (real32)state->dt;
    } else {
      state->heading_opacity -= state->heading_fadeout_duration * (real32)state->dt;
    }
  }

  {
    GuiContainer *container = state->gui_manager.make_container(
      "Peony debug info", glm::vec2(25.0f, 25.0f)
    );

    sprintf(debug_text, "%d fps", state->last_fps);
    state->gui_manager.draw_named_value(
      container, "fps", debug_text
    );

    sprintf(debug_text, "%.2f ms", state->dt_average * 1000.0f);
    state->gui_manager.draw_named_value(
      container, "dt", debug_text
    );

    sprintf(debug_text, "%d", state->entities.size);
    state->gui_manager.draw_named_value(
      container, "entities.size", debug_text
    );

    if (state->gui_manager.draw_toggle(
      container, "Wireframe mode", &state->should_use_wireframe
    )) {
      state->should_use_wireframe = !state->should_use_wireframe;
      if (state->should_use_wireframe) {
        set_heading(state, "Wireframe mode on.", 1.0f, 1.0f, 1.0f);
      } else {
        set_heading(state, "Wireframe mode off.", 1.0f, 1.0f, 1.0f);
      }
    }

    if (state->gui_manager.draw_toggle(
      container, "FPS limit", &state->should_limit_fps
    )) {
      state->should_limit_fps = !state->should_limit_fps;
      if (state->should_limit_fps) {
        set_heading(state, "FPS limit enabled.", 1.0f, 1.0f, 1.0f);
      } else {
        set_heading(state, "FPS limit disabled.", 1.0f, 1.0f, 1.0f);
      }
    }

    if (state->gui_manager.draw_toggle(
      container, "Manual frame advance", &state->is_manual_frame_advance_enabled
    )) {
      state->is_manual_frame_advance_enabled = !state->is_manual_frame_advance_enabled;
      if (state->is_manual_frame_advance_enabled) {
        set_heading(state, "Manual frame advance enabled.", 1.0f, 1.0f, 1.0f);
      } else {
        set_heading(state, "Manual frame advance disabled.", 1.0f, 1.0f, 1.0f);
      }
    }

    if (state->gui_manager.draw_toggle(
      container, "Pause", &state->should_pause
    )) {
      state->should_pause = !state->should_pause;
      if (state->should_pause) {
        set_heading(state, "Pause enabled.", 1.0f, 1.0f, 1.0f);
      } else {
        set_heading(state, "Pause disabled.", 1.0f, 1.0f, 1.0f);
      }
    }

    if (state->gui_manager.draw_button(
      container, "Reload shaders"
    )) {
      reload_shaders(memory, state);
      set_heading(state, "Shaders reloaded.", 1.0f, 1.0f, 1.0f);
    }

    if (state->gui_manager.draw_button(
      container, "Delete PBO"
    )) {
      state->persistent_pbo.delete_pbo();
      set_heading(state, "PBO deleted.", 1.0f, 1.0f, 1.0f);
    }
  }

  {
#if 1
    GuiContainer *container = state->gui_manager.make_container(
      "Entities", glm::vec2(state->window_info.width - 400.0f, 25.0f)
    );
    get_scene_text_representation(debug_text, state);
    state->gui_manager.draw_body_text(container, debug_text);
#endif
  }

  state->gui_manager.render();
}


void scene_update(Memory *memory, State *state) {
  for (
    uint32 idx = 1;
    idx < state->behavior_component_manager.components->size;
    idx++
  ) {
    BehaviorComponent *behavior_component =
      state->behavior_component_manager.components->get(idx);

    if (
      !behavior_component ||
      !behavior_component->is_valid()
    ) {
      continue;
    }

    EntityHandle entity_handle = behavior_component->entity_handle;

    SpatialComponent *spatial_component =
      state->spatial_component_manager.get(entity_handle);
    if (!spatial_component) {
      log_error("Could not get SpatialComponent for BehaviorComponent");
      continue;
    }

    Entity *entity = state->entities.get(entity_handle);
    if (!entity) {
      log_error("Could not get Entity for BehaviorComponent");
      continue;
    }

    if (behavior_component->behavior == Behavior::test) {
      spatial_component->position = glm::vec3(
        (real32)sin(state->t) * 15.0f,
        (real32)((sin(state->t * 2.0f) + 1.5) * 3.0f),
        (real32)cos(state->t) * 15.0f
      );
    }
  }

  {
    for (uint32 idx = 0; idx < state->light_component_manager.components->size; idx++) {
      LightComponent *light_component =
        state->light_component_manager.components->get(idx);
      SpatialComponent *spatial_component = state->spatial_component_manager.get(
        light_component->entity_handle
      );

      if (!(
        light_component->is_valid() &&
        spatial_component->is_valid()
      )) {
        continue;
      }

      if (light_component->type == LightType::point) {
        real64 time_term =
          (sin(state->t / 1.5f) + 1.0f) / 2.0f * (PI / 2.0f) + (PI / 2.0f);
        real64 x_term = 0.0f + cos(time_term) * 8.0f;
        real64 z_term = 0.0f + sin(time_term) * 8.0f;
        spatial_component->position.x = (real32)x_term;
        spatial_component->position.z = (real32)z_term;
      } else if (light_component->type == LightType::directional) {
        spatial_component->position = state->camera_active->position +
          -light_component->direction * DIRECTIONAL_LIGHT_DISTANCE;
      }
    }
  }
}


void check_all_model_assets_loaded(Memory *memory, State *state) {
  for (uint32 idx = 0; idx < state->model_assets.size; idx++) {
    ModelAsset *model_asset = state->model_assets[idx];
    model_asset->prepare_for_draw(
      memory,
      &state->persistent_pbo,
      &state->texture_name_pool,
      &state->task_queue
    );
  }
}


void update_and_render(Memory *memory, State *state) {
  state->camera_active->update_matrices(
    state->window_info.width, state->window_info.height
  );
  check_all_model_assets_loaded(memory, state);
  scene_update(memory, state);
  copy_scene_data_to_ubo(memory, state);

  // Clear framebuffers
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, state->g_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, state->l_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, state->blur1_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glBindFramebuffer(GL_FRAMEBUFFER, state->blur2_buffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  }

  // Render shadow map
  {
    // Point lights
    {
      glm::mat4 perspective_projection = glm::perspective(
        glm::radians(90.0f),
        (real32)state->cube_shadowmap_width / (real32)state->cube_shadowmap_height,
        state->shadowmap_near_clip_dist, state->shadowmap_far_clip_dist
      );

      uint32 idx_light = 0;

      for (uint32 idx = 0; idx < state->light_component_manager.components->size; idx++) {
        LightComponent *light_component =
          state->light_component_manager.components->get(idx);
        SpatialComponent *spatial_component = state->spatial_component_manager.get(
          light_component->entity_handle
        );

        if (!(
          light_component->is_valid() &&
          light_component->type == LightType::point &&
          spatial_component->is_valid()
        )) {
          continue;
        }

        glm::vec3 position = spatial_component->position;

        for (uint32 idx_face = 0; idx_face < 6; idx_face++) {
          state->cube_shadowmap_transforms[(idx_light * 6) + idx_face] =
            perspective_projection * glm::lookAt(
              position,
              position + CUBEMAP_OFFSETS[idx_face],
              CUBEMAP_UPS[idx_face]
            );
        }

        glViewport(0, 0, state->cube_shadowmap_width, state->cube_shadowmap_height);
        glBindFramebuffer(GL_FRAMEBUFFER, state->cube_shadowmaps_framebuffer);
        glClear(GL_DEPTH_BUFFER_BIT);

        copy_scene_data_to_ubo(
          memory, state, idx_light, light_type_to_int(light_component->type), false
        );
        render_scene(memory, state, RenderPass::shadowcaster, RenderMode::depth);

        idx_light++;
      }
    }

    // Directional lights
    {
      real32 ortho_ratio = (real32)state->texture_shadowmap_width /
        (real32)state->texture_shadowmap_height;
      real32 ortho_width = 100.0f;
      real32 ortho_height = ortho_width / ortho_ratio;
      glm::mat4 ortho_projection = glm::ortho(
        -ortho_width, ortho_width,
        -ortho_height, ortho_height,
        state->shadowmap_near_clip_dist, state->shadowmap_far_clip_dist
      );

      uint32 idx_light = 0;

      for (uint32 idx = 0; idx < state->light_component_manager.components->size; idx++) {
        LightComponent *light_component =
          state->light_component_manager.components->get(idx);
        SpatialComponent *spatial_component = state->spatial_component_manager.get(
          light_component->entity_handle
        );

        if (!(
          light_component->is_valid() &&
          light_component->type == LightType::directional &&
          spatial_component->is_valid()
        )) {
          continue;
        }

        state->texture_shadowmap_transforms[idx_light] = ortho_projection * glm::lookAt(
          spatial_component->position,
          spatial_component->position + light_component->direction,
          glm::vec3(0.0f, -1.0f, 0.0f)
        );

        glViewport(
          0, 0, state->texture_shadowmap_width, state->texture_shadowmap_height
        );
        glBindFramebuffer(GL_FRAMEBUFFER, state->texture_shadowmaps_framebuffer);
        glClear(GL_DEPTH_BUFFER_BIT);

        copy_scene_data_to_ubo(
          memory, state, idx_light, light_type_to_int(light_component->type), false
        );
        render_scene(memory, state, RenderPass::shadowcaster, RenderMode::depth);

        idx_light++;
      }
    }
  }

  glViewport(
    0, 0, state->window_info.width, state->window_info.height
  );

  // Geometry pass
  {
    if (state->should_use_wireframe) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, state->g_buffer);
    render_scene(memory, state, RenderPass::deferred, RenderMode::regular);
    if (state->should_use_wireframe) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
  }

  // Copy depth from geometry pass to lighting pass
  {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, state->g_buffer);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, state->l_buffer);
    glBlitFramebuffer(
      0, 0, state->window_info.width, state->window_info.height,
      0, 0, state->window_info.width, state->window_info.height,
      GL_DEPTH_BUFFER_BIT, GL_NEAREST
    );
  }

  glBindFramebuffer(GL_FRAMEBUFFER, state->l_buffer);

  // Lighting pass
  {
    glDisable(GL_DEPTH_TEST);
    render_scene(memory, state, RenderPass::lighting, RenderMode::regular);
    glEnable(GL_DEPTH_TEST);
  }


  // Forward pass
  {
    // Skysphere
    {
      // Cull outside, not inside, of sphere.
      glCullFace(GL_FRONT);
      // Do not write to depth buffer.
      glDepthMask(GL_FALSE);
      // Draw at the very back of our depth range, so as to be behind everything.
      glDepthRange(0.9999f, 1.0f);

      render_scene(memory, state, RenderPass::forward_skybox, RenderMode::regular);

      glDepthRange(0.0f, 1.0f);
      glDepthMask(GL_TRUE);
      glCullFace(GL_BACK);
    }

    // Forward
    {
      if (state->should_use_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      }

      render_scene(memory, state, RenderPass::forward_depth, RenderMode::regular);

      glDisable(GL_DEPTH_TEST);
      render_scene(memory, state, RenderPass::forward_nodepth, RenderMode::regular);
      glEnable(GL_DEPTH_TEST);

      if (state->should_use_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      }
    }
  }

  glDisable(GL_DEPTH_TEST);

  // Blur pass
  {
    glBindFramebuffer(GL_FRAMEBUFFER, state->blur1_buffer);
    copy_scene_data_to_ubo(memory, state, 0, 0, true);
    render_scene(memory, state, RenderPass::preblur, RenderMode::regular);

    glBindFramebuffer(GL_FRAMEBUFFER, state->blur2_buffer);
    copy_scene_data_to_ubo(memory, state, 0, 0, false);
    render_scene(memory, state, RenderPass::blur2, RenderMode::regular);

    for (uint32 idx = 0; idx < 3; idx++) {
      glBindFramebuffer(GL_FRAMEBUFFER, state->blur1_buffer);
      copy_scene_data_to_ubo(memory, state, 0, 0, true);
      render_scene(memory, state, RenderPass::blur1, RenderMode::regular);

      glBindFramebuffer(GL_FRAMEBUFFER, state->blur2_buffer);
      copy_scene_data_to_ubo(memory, state, 0, 0, false);
      render_scene(memory, state, RenderPass::blur1, RenderMode::regular);
    }
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Postprocessing pass
  {
    render_scene(memory, state, RenderPass::postprocessing, RenderMode::regular);
  }

  // UI pass
  {
    glEnable(GL_BLEND);
    if (!state->should_hide_ui) {
      render_scene_ui(memory, state);
    }
    glDisable(GL_BLEND);
  }

  glEnable(GL_DEPTH_TEST);
}


void run_main_loop(Memory *memory, State *state) {
  std::chrono::steady_clock::time_point second_start = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point frame_start = std::chrono::steady_clock::now();
  std::chrono::steady_clock::time_point last_frame_start = std::chrono::steady_clock::now();
  // 1/165 seconds (for 165 fps)
  std::chrono::nanoseconds frame_duration = std::chrono::nanoseconds(6060606);
  std::chrono::steady_clock::time_point time_frame_should_end;
  uint32 n_frames_this_second = 0;
  state->n_frames_since_start = 0;

  while (!state->should_stop) {
    glfwPollEvents();
    process_input(state->window_info.window, state, memory);

    if (
      !state->is_manual_frame_advance_enabled ||
      state->should_manually_advance_to_next_frame
    ) {
      state->n_frames_since_start++;
      last_frame_start = frame_start;
      frame_start = std::chrono::steady_clock::now();
      time_frame_should_end = frame_start + frame_duration;

      // If we should pause, stop time-based events.
      if (!state->should_pause) {
        state->dt = std::chrono::duration_cast<std::chrono::duration<real64>>(
          frame_start - last_frame_start
        ).count();

        state->dt_hist[state->dt_hist_idx] = state->dt;
        state->dt_hist_idx++;
        if (state->dt_hist_idx >= DT_HIST_LENGTH) {
          state->dt_hist_idx = 0;
        }
        real64 dt_hist_sum = 0.0f;
        for (uint32 idx = 0; idx < DT_HIST_LENGTH; idx++) {
          dt_hist_sum += state->dt_hist[idx];
        }
        state->dt_average = dt_hist_sum / DT_HIST_LENGTH;

        state->t += state->dt;
      }

      // Count FPS.
      n_frames_this_second++;
      std::chrono::duration<real64> time_since_second_start = frame_start - second_start;
      if (time_since_second_start >= std::chrono::seconds(1)) {
        second_start = frame_start;
        state->last_fps = n_frames_this_second;
        n_frames_this_second = 0;
        if (state->should_hide_ui) {
          log_info("%.2f ms", state->dt_average * 1000.0f);
        }
      }

      // NOTE: Don't render on the very first frame. This avoids flashing that happens in
      // fullscreen. There is a better way to handle this, but whatever, figure it out later.
      if (state->n_frames_since_start > 1) {
        update_and_render(memory, state);
      }
      if (state->is_manual_frame_advance_enabled) {
        state->should_manually_advance_to_next_frame = false;
      }
      state->input_manager.reset_n_mouse_button_state_changes_this_frame();
      state->input_manager.reset_n_key_state_changes_this_frame();

      if (state->should_limit_fps) {
        std::this_thread::sleep_until(time_frame_should_end);
      }
    }

    START_TIMER(swap_buffers);
    glfwSwapBuffers(state->window_info.window);
    END_TIMER_MIN(swap_buffers, 16);

    if (glfwWindowShouldClose(state->window_info.window)) {
      state->should_stop = true;
    }
  }
}


void destroy_window() {
  glfwTerminate();
}


void run_loading_loop(
  std::mutex *mutex, Memory *memory, State *state, uint32 idx_thread
) {
  while (!state->should_stop) {
    Task *task = nullptr;

    mutex->lock();
    if (state->task_queue.size > 0) {
      task = state->task_queue.pop();
    }
    mutex->unlock();

    if (task) {
      log_info(
        "[Thread #%d] Running task %s for model %s",
        idx_thread,
        Task::task_type_to_str(task->type),
        task->model_asset->name
      );
      task->run();
      log_info(
        "[Thread #%d] Finished task %s for model %s",
        idx_thread,
        Task::task_type_to_str(task->type),
        task->model_asset->name
      );
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}


void check_environment() {
  // Check that an `enum class`'s default value == its first element == 0;
  BehaviorComponent test;
  assert(test.behavior == Behavior::none);
}


int main() {
  check_environment();

  START_TIMER(init);

  srand((uint32)time(NULL));
  START_TIMER(allocate_memory);
  Memory memory;
  END_TIMER(allocate_memory);

  WindowInfo window_info;
  START_TIMER(init_window);
  init_window(&window_info);
  END_TIMER(init_window);
  if (!window_info.window) {
    return -1;
  }

  State *state = new((State*)memory.state_memory) State(&memory, window_info);

  ModelAsset::entity_manager = &state->entity_manager;
  ModelAsset::drawable_component_manager = &state->drawable_component_manager;
  ModelAsset::spatial_component_manager = &state->spatial_component_manager;
  ModelAsset::light_component_manager = &state->light_component_manager;
  ModelAsset::behavior_component_manager = &state->behavior_component_manager;

  std::mutex loading_thread_mutex;
  std::thread loading_thread_1 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state, 1);
  std::thread loading_thread_2 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state, 2);
  std::thread loading_thread_3 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state, 3);
  std::thread loading_thread_4 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state, 4);
  std::thread loading_thread_5 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state, 5);

#if 0
  Util::print_texture_internalformat_info(GL_RGB8);
  Util::print_texture_internalformat_info(GL_RGBA8);
  Util::print_texture_internalformat_info(GL_SRGB8);
#endif

  update_drawing_options(state, window_info.window);

  MemoryAndState memory_and_state = {&memory, state};
  glfwSetWindowUserPointer(window_info.window, &memory_and_state);

  state->texture_name_pool.allocate_texture_names();
  init_g_buffer(&memory, state);
  init_l_buffer(&memory, state);
  init_blur_buffers(&memory, state);
  init_shadowmaps(&memory, state);
  init_ubo(&memory, state);
  World::init(&memory, state);
  state->persistent_pbo.allocate_pbo();
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

#if 0
  memory.asset_memory_pool.print();
  memory.entity_memory_pool.print();
  memory.temp_memory_pool.print();
#endif

#if USE_CACHELINE_SIZE_DISPLAY
  log_info("Cache line size: %dB", cacheline_get_size());
#endif

  END_TIMER(init);

  run_main_loop(&memory, state);

  loading_thread_1.join();
  loading_thread_2.join();
  loading_thread_3.join();
  loading_thread_4.join();
  loading_thread_5.join();

  destroy_window();

  log_info("Bye!");

  return 0;
}
