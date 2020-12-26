#define _CRT_SECURE_NO_WARNINGS

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
#include "util.cpp"
#include "task.cpp"
#include "peony_file_parser.cpp"
#include "textures.cpp"
#include "font_asset.cpp"
#include "shader_asset.cpp"
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
#include "renderer.cpp"


void reload_shaders(Memory *memory, State *state) {
  for (uint32 idx = 0; idx < state->shader_assets.size; idx++) {
    ShaderAsset *shader_asset = state->shader_assets[idx];
    shader_asset->load(memory);
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
    Renderer::update_drawing_options(state, window);
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


void render_scene(
  Memory *memory,
  State *state,
  Renderer::RenderPassFlag render_pass,
  Renderer::RenderMode render_mode
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
      Textures::delete_persistent_pbo(&state->persistent_pbo);
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


void scene_update(Memory *memory, State *state) {
  state->camera_active->update_matrices(
    state->window_info.width, state->window_info.height
  );
  check_all_model_assets_loaded(memory, state);

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


void render(Memory *memory, State *state) {
  Renderer::copy_scene_data_to_ubo(memory, state);

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

        Renderer::copy_scene_data_to_ubo(
          memory, state, idx_light, light_type_to_int(light_component->type), false
        );
        render_scene(
          memory, state,
          Renderer::RenderPass::shadowcaster,
          Renderer::RenderMode::depth
        );

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

        Renderer::copy_scene_data_to_ubo(
          memory, state, idx_light, light_type_to_int(light_component->type), false
        );
        render_scene(
          memory, state,
          Renderer::RenderPass::shadowcaster,
          Renderer::RenderMode::depth
        );

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
    render_scene(
      memory, state,
      Renderer::RenderPass::deferred,
      Renderer::RenderMode::regular
    );
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
    render_scene(
      memory, state,
      Renderer::RenderPass::lighting,
      Renderer::RenderMode::regular
    );
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

      render_scene(
        memory, state,
        Renderer::RenderPass::forward_skybox,
        Renderer::RenderMode::regular
      );

      glDepthRange(0.0f, 1.0f);
      glDepthMask(GL_TRUE);
      glCullFace(GL_BACK);
    }

    // Forward
    {
      if (state->should_use_wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
      }

      render_scene(
        memory, state,
        Renderer::RenderPass::forward_depth,
        Renderer::RenderMode::regular
      );

      glDisable(GL_DEPTH_TEST);
      render_scene(
        memory, state,
        Renderer::RenderPass::forward_nodepth,
        Renderer::RenderMode::regular
      );
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
    Renderer::copy_scene_data_to_ubo(memory, state, 0, 0, true);
    render_scene(
      memory, state, Renderer::RenderPass::preblur, Renderer::RenderMode::regular
    );

    glBindFramebuffer(GL_FRAMEBUFFER, state->blur2_buffer);
    Renderer::copy_scene_data_to_ubo(memory, state, 0, 0, false);
    render_scene(
      memory, state, Renderer::RenderPass::blur2, Renderer::RenderMode::regular
    );

    for (uint32 idx = 0; idx < 3; idx++) {
      glBindFramebuffer(GL_FRAMEBUFFER, state->blur1_buffer);
      Renderer::copy_scene_data_to_ubo(memory, state, 0, 0, true);
      render_scene(
        memory, state, Renderer::RenderPass::blur1, Renderer::RenderMode::regular
      );

      glBindFramebuffer(GL_FRAMEBUFFER, state->blur2_buffer);
      Renderer::copy_scene_data_to_ubo(memory, state, 0, 0, false);
      render_scene(
        memory, state, Renderer::RenderPass::blur1, Renderer::RenderMode::regular
      );
    }
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // Postprocessing pass
  {
    render_scene(
      memory, state,
      Renderer::RenderPass::postprocessing,
      Renderer::RenderMode::regular
    );
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
        scene_update(memory, state);
        render(memory, state);
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
  Renderer::init_window(&window_info);
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

  Renderer::update_drawing_options(state, window_info.window);

  MemoryAndState memory_and_state = {&memory, state};
  glfwSetWindowUserPointer(window_info.window, &memory_and_state);

  Textures::init_texture_name_pool(&state->texture_name_pool, &memory, 64, 4);
  Renderer::init_g_buffer(&memory, state);
  Renderer::init_l_buffer(&memory, state);
  Renderer::init_blur_buffers(&memory, state);
  Renderer::init_shadowmaps(&memory, state);
  Renderer::init_ubo(&memory, state);
  World::init(&memory, state);
  Textures::init_persistent_pbo(&state->persistent_pbo, 25, 2048, 2048, 4);

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
