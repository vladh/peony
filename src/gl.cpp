#define _CRT_SECURE_NO_WARNINGS

#define USE_OPENGL_DEBUG false
#define USE_TIMERS true

#include "gl.hpp"

global_variable uint32 global_oopses = 0;

#include "log.cpp"
#include "pack.cpp"
#include "util.cpp"
#include "task.cpp"
#include "resource_manager.cpp"
#include "texture_name_pool.cpp"
#include "font_asset.cpp"
#include "shader_asset.cpp"
#include "persistent_pbo.cpp"
#include "texture_set.cpp"
#include "camera.cpp"
#include "memory.cpp"
#include "control.cpp"
#include "entity.cpp"
#include "entity_manager.cpp"
#include "drawable_component_manager.cpp"
#include "light_component_manager.cpp"
#include "spatial_component_manager.cpp"
#include "gui_manager.cpp"
#include "model_asset.cpp"
#include "scene.cpp"
#include "scene_resources.cpp"
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
    GL_TEXTURE_2D, TEXTURE_G_POSITION, g_position_texture_name,
    state->window_info.width, state->window_info.height, 4
  );
  state->g_normal_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "g_normal_texture")
  ) Texture(
    GL_TEXTURE_2D, TEXTURE_G_NORMAL, g_normal_texture_name,
    state->window_info.width, state->window_info.height, 4
  );
  state->g_albedo_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "g_albedo_texture")
  ) Texture(
    GL_TEXTURE_2D, TEXTURE_G_ALBEDO, g_albedo_texture_name,
    state->window_info.width, state->window_info.height, 4
  );
  state->g_pbr_texture = new(
    (Texture*)memory->asset_memory_pool.push(sizeof(Texture), "g_pbr_texture")
  ) Texture(
    GL_TEXTURE_2D, TEXTURE_G_PBR, g_pbr_texture_name,
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
    log_error("Framebuffer not complete!");
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
    GL_TEXTURE_2D, TEXTURE_L_COLOR, l_color_texture_name,
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
    GL_TEXTURE_2D, TEXTURE_L_BRIGHT_COLOR, l_bright_color_texture_name,
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
    GL_TEXTURE_2D, TEXTURE_L_DEPTH, l_depth_texture_name,
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
    log_error("Framebuffer not complete!");
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
    GL_TEXTURE_2D, TEXTURE_BLUR1, blur1_texture_name,
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
    GL_TEXTURE_2D, TEXTURE_BLUR2, blur2_texture_name,
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
}


void reload_shaders(Memory *memory, State *state) {
  for (uint32 idx = 0; idx < state->shader_assets.size; idx++) {
    ShaderAsset *shader_asset = state->shader_assets.get(idx);
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
  for (uint32 idx = 0; idx < state->lights.size; idx++) {
    EntityHandle *handle = state->lights.get(idx);

    if (handle) {
      LightComponent *light_component = state->light_component_manager.get(*handle);
      if (light_component->type == LIGHT_DIRECTIONAL) {
        state->dir_light_angle += amount;
        light_component->direction = glm::vec3(
          sin(state->dir_light_angle), -cos(state->dir_light_angle), 0.0f
        );
      }
    }
  }
}


void process_input_continuous(GLFWwindow *window, State *state, Memory *memory) {
  if (state->control.is_key_down(GLFW_KEY_W)) {
    state->camera_active->move_front_back(1, state->dt);
  }

  if (state->control.is_key_down(GLFW_KEY_S)) {
    state->camera_active->move_front_back(-1, state->dt);
  }

  if (state->control.is_key_down(GLFW_KEY_A)) {
    state->camera_active->move_left_right(-1, state->dt);
  }

  if (state->control.is_key_down(GLFW_KEY_D)) {
    state->camera_active->move_left_right(1, state->dt);
  }

  if (state->control.is_key_down(GLFW_KEY_Z)) {
    update_light_position(state, 0.10f * (real32)state->dt);
  }

  if (state->control.is_key_down(GLFW_KEY_X)) {
    update_light_position(state, -0.10f * (real32)state->dt);
  }

  if (state->control.is_key_down(GLFW_KEY_SPACE)) {
    state->camera_active->move_up_down(1, state->dt);
  }

  if (state->control.is_key_down(GLFW_KEY_LEFT_CONTROL)) {
    state->camera_active->move_up_down(-1, state->dt);
  }
}


void process_input_transient(GLFWwindow *window, State *state, Memory *memory) {
  if (state->control.is_key_now_down(GLFW_KEY_ESCAPE)) {
    glfwSetWindowShouldClose(window, true);
  }

  if (state->control.is_key_now_down(GLFW_KEY_F)) {
    state->should_limit_fps = !state->should_limit_fps;
    update_drawing_options(state, window);
  }

  if (state->control.is_key_now_down(GLFW_KEY_C)) {
    state->is_cursor_disabled = !state->is_cursor_disabled;
    update_drawing_options(state, window);
  }

  if (state->control.is_key_now_down(GLFW_KEY_Q)) {
    state->should_use_wireframe = !state->should_use_wireframe;
    update_drawing_options(state, window);
  }

  if (state->control.is_key_now_down(GLFW_KEY_TAB)) {
    state->should_pause = !state->should_pause;
  }

  if (state->control.is_key_now_down(GLFW_KEY_BACKSPACE)) {
    state->should_hide_ui = !state->should_hide_ui;
  }

  if (state->control.is_key_now_down(GLFW_KEY_GRAVE_ACCENT)) {
    log_info("Deleting PBO");
    state->persistent_pbo.delete_pbo();
  }

  if (state->control.is_key_now_down(GLFW_KEY_R)) {
    reload_shaders(memory, state);
  }

  if (state->control.is_key_down(GLFW_KEY_P)) {
    state->is_manual_frame_advance_enabled = !state->is_manual_frame_advance_enabled;
  }

  if (state->control.is_key_down(GLFW_KEY_ENTER)) {
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

  // TODO: Only regenerate once we're done resizing, not for every little bit of the resize.
  init_g_buffer(memory, state);
  init_l_buffer(memory, state);
  init_blur_buffers(memory, state);

  // TODO: I hate this.
  for (
    uint32 idx_component = 0;
    idx_component < state->drawable_component_manager.components->size;
    idx_component++
  ) {
    DrawableComponent *component = state->drawable_component_manager.components->get(
      idx_component
    );
    ModelAsset *model_asset = component->model_asset;
    for (
      uint32 idx_mesh = 0; idx_mesh < model_asset->meshes.size; idx_mesh++
    ) {
      Mesh *mesh = model_asset->meshes.get(idx_mesh);
      if (mesh->texture_set && mesh->texture_set->is_screensize_dependent) {
        log_info("Found G-buffer dependent mesh in model %s", model_asset->name);
        for(
          uint32 idx_texture = 0; idx_texture < mesh->texture_set->textures.size; idx_texture++
        ) {
          Texture *texture = mesh->texture_set->textures.get(idx_texture);
          if (texture->type == TEXTURE_G_POSITION) {
            mesh->texture_set->textures.set(idx_texture, state->g_position_texture);
          } else if (texture->type == TEXTURE_G_NORMAL) {
            mesh->texture_set->textures.set(idx_texture, state->g_normal_texture);
          } else if (texture->type == TEXTURE_G_ALBEDO) {
            mesh->texture_set->textures.set(idx_texture, state->g_albedo_texture);
          } else if (texture->type == TEXTURE_G_PBR) {
            mesh->texture_set->textures.set(idx_texture, state->g_pbr_texture);
          } else if (texture->type == TEXTURE_L_COLOR) {
            mesh->texture_set->textures.set(idx_texture, state->l_color_texture);
          } else if (texture->type == TEXTURE_L_BRIGHT_COLOR) {
            mesh->texture_set->textures.set(idx_texture, state->l_bright_color_texture);
          } else if (texture->type == TEXTURE_L_DEPTH) {
            mesh->texture_set->textures.set(idx_texture, state->l_depth_texture);
          } else if (texture->type == TEXTURE_BLUR1) {
            mesh->texture_set->textures.set(idx_texture, state->blur1_texture);
          } else if (texture->type == TEXTURE_BLUR2) {
            mesh->texture_set->textures.set(idx_texture, state->blur2_texture);
          }
        }
        model_asset->bind_texture_uniforms_for_mesh(mesh);
      }
    }
  }
}


void mouse_callback(GLFWwindow *window, real64 x, real64 y) {
  MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
  State *state = memory_and_state->state;
  glm::vec2 mouse_offset = state->control.update_mouse(x, y);
  state->camera_active->update_mouse(mouse_offset);
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  MemoryAndState *memory_and_state = (MemoryAndState*)glfwGetWindowUserPointer(window);
  State *state = memory_and_state->state;
  Memory *memory = memory_and_state->memory;
  state->control.update_keys(key, scancode, action, mods);
  process_input_transient(window, state, memory);
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
  GLFWmonitor *target_monitor = monitors[1];

  const GLFWvidmode *video_mode = glfwGetVideoMode(target_monitor);
  glfwWindowHint(GLFW_RED_BITS, video_mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, video_mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, video_mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, video_mode->refreshRate);
  window_info->width = video_mode->width;
  window_info->height = video_mode->height;
  /* window_info->width = 800; */
  /* window_info->height = 600; */

  GLFWwindow *window = glfwCreateWindow(
    window_info->width, window_info->height, window_info->title,
    target_monitor, nullptr
    /* nullptr, nullptr */
  );
  if (!window) {
    log_fatal("Failed to create GLFW window");
    return;
  }
  window_info->window = window;
  glfwSetWindowPos(window, 0, 0);
  /* glfwSetWindowPos(window, 200, 200); */

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
  glfwSetKeyCallback(window, key_callback);
}


void copy_scene_data_to_ubo(
  Memory *memory, State *state, uint32 shadow_light_idx, bool32 is_blur_horizontal
) {
  ShaderCommon *shader_common = &state->shader_common;

  shader_common->view = state->camera_active->view;
  shader_common->projection = state->camera_active->projection;
  shader_common->ui_projection = state->camera_active->ui_projection;
  memcpy(shader_common->shadow_transforms, state->shadow_transforms, sizeof(state->shadow_transforms));
  shader_common->camera_position = glm::vec4(state->camera_active->position, 1.0f);
  shader_common->camera_pitch = (float)state->camera_active->pitch;

  shader_common->camera_horizontal_fov = state->camera_active->horizontal_fov;
  shader_common->camera_vertical_fov = state->camera_active->vertical_fov;
  shader_common->camera_near_clip_dist = state->camera_active->near_clip_dist;
  shader_common->camera_far_clip_dist = state->camera_active->far_clip_dist;

  shader_common->n_lights = state->lights.size;
  shader_common->shadow_light_idx = shadow_light_idx;
  shader_common->shadow_far_clip_dist = state->shadowmap_far_clip_dist;
  shader_common->is_blur_horizontal = is_blur_horizontal;

  shader_common->exposure = state->camera_active->exposure;
  shader_common->t = (float)state->t;
  shader_common->window_width = state->window_info.width;
  shader_common->window_height = state->window_info.height;

  for (uint32 idx = 0; idx < state->lights.size; idx++) {
    EntityHandle handle = *state->lights.get(idx);
    SpatialComponent *spatial_component = state->spatial_component_manager.get(handle);
    LightComponent *light_component = state->light_component_manager.get(handle);
    shader_common->light_position[idx] = glm::vec4(spatial_component->position, 1.0f);
    shader_common->light_type[idx] = glm::vec4(light_component->type, 0.0f, 0.0f, 0.0f);
    shader_common->light_direction[idx] = glm::vec4(light_component->direction, 1.0f);
    shader_common->light_color[idx] = light_component->color;
    shader_common->light_attenuation[idx] = light_component->attenuation;
  }

  glBindBuffer(GL_UNIFORM_BUFFER, state->ubo_shader_common);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ShaderCommon), shader_common);
}


void copy_scene_data_to_ubo(Memory *memory, State *state) {
  copy_scene_data_to_ubo(memory, state, 0, false);
}


void render_scene(
  Memory *memory, State *state, RenderPass render_pass, RenderMode render_mode
) {
  state->drawable_component_manager.draw_all(
    memory,
    &state->persistent_pbo,
    &state->texture_name_pool,
    &state->spatial_component_manager,
    &state->task_queue,
    render_pass, render_mode, state->standard_depth_shader_asset
  );
}


void render_scene_ui(
  Memory *memory, State *state
){
  char debug_text[256];
  sprintf(
    debug_text,
    "(fps %.2f)\n"
    "(t %f)\n"
    "(dt %f)\n"
    "(frame %d)\n"
    "(should_limit_fps %d)\n"
    "(pitch %f)\n"
    "(oopses %d)",
    state->last_fps,
    state->t,
    state->dt,
    state->n_frames_since_start,
    state->should_limit_fps,
    state->camera_active->pitch,
    global_oopses
  );
  state->gui_manager.draw_text(
    "main-font", debug_text,
    15.0f, state->window_info.height - 35.0f,
    1.0f, glm::vec4(0.00f, 0.33f, 0.93f, 0.5f)
  );
}


void scene_update(Memory *memory, State *state) {
  // TODO: Eventually move this to some kind of ActorComponent system.
  // We should rather be iterating through all SpatialComponents rather
  // than looking everything up.

  // Lights
  {
#if 1
    for (uint32 idx = 0; idx < state->lights.size; idx++) {
      EntityHandle *handle = state->lights.get(idx);

      if (handle) {
        SpatialComponent *spatial_component = state->spatial_component_manager.get(*handle);
        LightComponent *light_component = state->light_component_manager.get(*handle);

        if (spatial_component && light_component->type == LIGHT_POINT) {
          real64 time_term =
            (sin(state->t / 1.5f) + 1.0f) / 2.0f * (PI / 2.0f) + (PI / 2.0f);
          real64 x_term = 0.0f + cos(time_term) * 8.0f;
          real64 z_term = 0.0f + sin(time_term) * 8.0f;

          spatial_component->position.x = (real32)x_term;
          spatial_component->position.z = (real32)z_term;
        } else if (spatial_component && light_component->type == LIGHT_DIRECTIONAL) {
          spatial_component->position = state->camera_active->position +
            -light_component->direction * DIRECTIONAL_LIGHT_DISTANCE;
        }
      }
    }
#endif
  }

  // Water
  {
#if 0
    if (state->ocean) {
      SpatialComponent *spatial_component = state->spatial_component_manager.get(state->ocean);
      spatial_component->position = glm::vec3(
        state->camera_active->position.x,
        0,
        state->camera_active->position.z
      );
    }
#endif
  }

  // Geese
#if 0
  {
    real32 spin_deg_per_t = 90.0f;
    for (uint32 idx = 0; idx < state->geese.size; idx++) {
      SpatialComponent *spatial = state->spatial_component_manager.get(*state->geese.get(idx));
      spatial->rotation *= glm::angleAxis(
        glm::radians(spin_deg_per_t * (real32)state->dt),
        glm::vec3(0.0f, 1.0f, 0.0f)
      );
    }
  }
#endif
}


void update_and_render(Memory *memory, State *state) {
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  state->camera_active->update_matrices(
    state->window_info.width, state->window_info.height
  );
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

    Camera::create_shadow_transforms(
      state->shadow_transforms,
      &state->spatial_component_manager,
      &state->light_component_manager,
      &state->lights,
      state->cube_shadowmap_width, state->cube_shadowmap_height,
      state->texture_shadowmap_width, state->texture_shadowmap_height,
      state->shadowmap_near_clip_dist, state->shadowmap_far_clip_dist
    );

    for (uint32 idx = 0; idx < state->lights.size; idx++) {
      EntityHandle handle = *state->lights.get(idx);
      LightComponent *light_component = state->light_component_manager.get(handle);

      // TODO: Keep separate indices into the two framebuffers to avoid wasting space.
      if (light_component->type == LIGHT_POINT) {
        glViewport(
          0, 0, state->cube_shadowmap_width, state->cube_shadowmap_height
        );
        glBindFramebuffer(GL_FRAMEBUFFER, state->cube_shadowmaps_framebuffer);
      } else if (light_component->type == LIGHT_DIRECTIONAL) {
        glViewport(
          0, 0, state->texture_shadowmap_width, state->texture_shadowmap_height
        );
        glBindFramebuffer(GL_FRAMEBUFFER, state->texture_shadowmaps_framebuffer);
      }
      glClear(GL_DEPTH_BUFFER_BIT);

      copy_scene_data_to_ubo(memory, state, idx, false);
      render_scene(
        memory, state, RENDERPASS_DEFERRED, RENDERMODE_DEPTH
      );
      render_scene(
        memory, state, RENDERPASS_FORWARD_DEPTH, RENDERMODE_DEPTH
      );
    }

    glViewport(
      0, 0, state->window_info.width, state->window_info.height
    );
  }

  // Geometry pass
  {
    if (state->should_use_wireframe) {
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, state->g_buffer);
    render_scene(memory, state, RENDERPASS_DEFERRED, RENDERMODE_REGULAR);
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
    render_scene(memory, state, RENDERPASS_LIGHTING, RENDERMODE_REGULAR);
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

      render_scene(memory, state, RENDERPASS_FORWARD_SKYBOX, RENDERMODE_REGULAR);

      glDepthRange(0.0f, 1.0f);
      glDepthMask(GL_TRUE);
      glCullFace(GL_BACK);
    }

    // Forward
    {
      if (state->should_use_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      }

      render_scene(memory, state, RENDERPASS_FORWARD_DEPTH, RENDERMODE_REGULAR);

      glDisable(GL_DEPTH_TEST);
      render_scene(memory, state, RENDERPASS_FORWARD_NODEPTH, RENDERMODE_REGULAR);
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
    copy_scene_data_to_ubo(memory, state, 0, true);
    render_scene(memory, state, RENDERPASS_PREBLUR, RENDERMODE_REGULAR);

    glBindFramebuffer(GL_FRAMEBUFFER, state->blur2_buffer);
    copy_scene_data_to_ubo(memory, state, 0, false);
    render_scene(memory, state, RENDERPASS_BLUR2, RENDERMODE_REGULAR);

    for (uint32 idx = 0; idx < 3; idx++) {
      glBindFramebuffer(GL_FRAMEBUFFER, state->blur1_buffer);
      copy_scene_data_to_ubo(memory, state, 0, true);
      render_scene(memory, state, RENDERPASS_BLUR1, RENDERMODE_REGULAR);

      glBindFramebuffer(GL_FRAMEBUFFER, state->blur2_buffer);
      copy_scene_data_to_ubo(memory, state, 0, false);
      render_scene(memory, state, RENDERPASS_BLUR1, RENDERMODE_REGULAR);
    }
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Postprocessing pass
  {
    render_scene(memory, state, RENDERPASS_POSTPROCESSING, RENDERMODE_REGULAR);
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
    process_input_continuous(state->window_info.window, state, memory);

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
        state->dt = std::chrono::duration_cast<std::chrono::duration<float>>(
          frame_start - last_frame_start
        ).count();
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
          log_info("%.2f FPS", state->last_fps);
        }
      }

      // TODO: Don't render on the very first frame. This avoids flashing that happens in
      // fullscreen. There is a better way to handle this, but whatever, figure it out later.
      if (state->n_frames_since_start > 1) {
        update_and_render(memory, state);
      }
      if (state->is_manual_frame_advance_enabled) {
        state->should_manually_advance_to_next_frame = false;
      }

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


void run_loading_loop(std::mutex *mutex, Memory *memory, State *state) {
  while (!state->should_stop) {
    Task *task = nullptr;

    mutex->lock();
    if (state->task_queue.size > 0) {
      task = state->task_queue.pop();
    }
    mutex->unlock();

    if (task) {
      task->run();
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}


int main() {
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

  std::mutex loading_thread_mutex;
  std::thread loading_thread_1 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state);
  std::thread loading_thread_2 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state);
  std::thread loading_thread_3 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state);
  std::thread loading_thread_4 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state);
  std::thread loading_thread_5 = std::thread(run_loading_loop, &loading_thread_mutex, &memory, state);

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
  scene_init_resources(&memory, state);
  scene_init_objects(&memory, state);
  state->persistent_pbo.allocate_pbo();

#if 0
  memory.asset_memory_pool.print();
  memory.entity_memory_pool.print();
  memory.temp_memory_pool.print();
#endif

  log_info("Cache line size: %dB", cacheline_get_size());

  END_TIMER(init);

  run_main_loop(&memory, state);

  loading_thread_1.join();
  loading_thread_2.join();
  loading_thread_3.join();
  loading_thread_4.join();
  loading_thread_5.join();

  destroy_window();
  return 0;
}
