#include "debug_ui.hpp"
#include "entities.hpp"
#include "intrinsics.hpp"


namespace debug_ui {
  pny_internal void get_entity_text_representation(
    char *text,
    State *state,
    Entity *entity,
    uint8 depth
  ) {
    char number[128];

    EntityHandle handle = entity->handle;
    SpatialComponent *spatial_component =
      state->spatial_component_set.components[handle];

    // Children will be drawn under their parents.
    if (
      depth == 0 &&
      spatial::is_spatial_component_valid(spatial_component) &&
      spatial_component->parent_entity_handle != entities::NO_ENTITY_HANDLE
    ) {
      return;
    }

    bool32 has_spatial_component = spatial::is_spatial_component_valid(
      spatial_component
    );
    bool32 has_drawable_component = models::is_drawable_component_valid(
      state->drawable_component_set.components[handle]
    );
    bool32 has_light_component = lights::is_light_component_valid(
      state->light_component_set.components[handle]
    );
    bool32 has_behavior_component = behavior::is_behavior_component_valid(
      state->behavior_component_set.components[handle]
    );
    bool32 has_animation_component = anim::is_animation_component_valid(
      state->animation_component_set.components[handle]
    );

    for (uint8 level = 0; level < depth; level++) {
      strcat(text, "  ");
    }

    strcat(text, "- ");
    strcat(text, entity->debug_name);

    strcat(text, "@");
    sprintf(text + strlen(text), "%d", entity->handle);

    if (
      !has_spatial_component &&
      !has_drawable_component &&
      !has_light_component &&
      !has_behavior_component &&
      !has_animation_component
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
    if (has_animation_component) {
      strcat(text, " +A");
    }

    if (spatial::is_spatial_component_valid(spatial_component)) {
      // NOTE: This is super slow lol.
      uint32 n_children_found = 0;
      each (child_spatial_component, state->spatial_component_set.components) {
        if (
          child_spatial_component->parent_entity_handle ==
            spatial_component->entity_handle
        ) {
          n_children_found++;
          if (n_children_found > 5) {
            continue;
          }
          EntityHandle child_handle = child_spatial_component->entity_handle;
          Entity *child_entity = state->entity_set.entities[child_handle];

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
        sprintf(
          text + strlen(text), "%d", n_children_found - 5
        );
        strcat(text, " more)");
      }
    }

    if (text[strlen(text) - 1] != '\n') {
      strcat(text, "\n");
    }
  }


  pny_internal void get_scene_text_representation(char *text, State *state) {
    text[0] = '\0';

    each (entity, state->entity_set.entities) {
      get_entity_text_representation(text, state, entity, 0);
    }

    if (text[strlen(text) - 1] == '\n') {
      text[strlen(text) - 1] = '\0';
    }
  }


  pny_internal void get_materials_text_representation(char *text, State *state) {
    text[0] = '\0';

    strcat(text, "Internal:\n");

    uint32 idx = 0;
    each (material, state->materials) {
      strcat(text, "- ");
      strcat(text, material->name);
      strcat(text, "\n");
      if (idx == state->first_non_internal_material_idx - 1) {
        strcat(text, "Non-internal: \n");
      }
      idx++;
    }

    if (text[strlen(text) - 1] == '\n') {
      text[strlen(text) - 1] = '\0';
    }
  }
}


void debug_ui::render_debug_ui(State *state) {
  char debug_text[1 << 14];
  size_t dt_size = sizeof(debug_text);

  gui::start_drawing(&state->gui_state);

  if (state->gui_state.heading_opacity > 0.0f) {
    gui::draw_heading(
      &state->gui_state,
      state->gui_state.heading_text,
      v4(0.0f, 0.33f, 0.93f, state->gui_state.heading_opacity)
    );
    if (state->gui_state.heading_fadeout_delay > 0.0f) {
      state->gui_state.heading_fadeout_delay -= (real32)state->dt;
    } else {
      state->gui_state.heading_opacity -=
        state->gui_state.heading_fadeout_duration * (real32)state->dt;
    }
  }

  {
    strcpy(debug_text, "Peony debug info: ");
    strcat(debug_text, state->current_scene_name);
    GuiContainer *container = gui::make_container(
      &state->gui_state, debug_text, v2(25.0f, 25.0f)
    );

    snprintf(
      debug_text, dt_size, "%ux%u", state->window_info.width, state->window_info.height
    );
    gui::draw_named_value(&state->gui_state, container, "screen size", debug_text);

    snprintf(
      debug_text, dt_size, "%ux%u",
      state->window_info.screencoord_width, state->window_info.screencoord_height
    );
    gui::draw_named_value(&state->gui_state, container, "window size", debug_text);

    snprintf(debug_text, dt_size, "%u fps", state->perf_counters.last_fps);
    gui::draw_named_value(&state->gui_state, container, "fps", debug_text);

    snprintf(debug_text, dt_size, "%.2f ms", state->perf_counters.dt_average * 1000.0f);
    gui::draw_named_value(&state->gui_state, container, "dt", debug_text);

    snprintf(debug_text, dt_size, "%.2f", 1.0f + state->timescale_diff);
    gui::draw_named_value(&state->gui_state, container, "ts", debug_text);

    snprintf(debug_text, dt_size, state->is_world_loaded ? "yes" : "no");
    gui::draw_named_value(&state->gui_state, container, "is_world_loaded", debug_text);

    snprintf(debug_text, dt_size, "%u", state->materials.length);
    gui::draw_named_value(
      &state->gui_state, container, "materials.length", debug_text
    );

    snprintf(debug_text, dt_size, "%u", state->entity_set.entities.length);
    gui::draw_named_value(&state->gui_state, container, "entities.length", debug_text);

    snprintf(debug_text, dt_size, "%u", state->model_loaders.length);
    gui::draw_named_value(
      &state->gui_state, container, "model_loaders.length", debug_text
    );

    snprintf(debug_text, dt_size, "%u", state->n_valid_model_loaders);
    gui::draw_named_value(
      &state->gui_state, container, "n_valid_model_loaders", debug_text
    );

    snprintf(debug_text, dt_size, "%u", state->entity_loader_set.loaders.length);
    gui::draw_named_value(
      &state->gui_state, container, "entity_loader_set.length", debug_text
    );

    snprintf(debug_text, dt_size, "%u", state->n_valid_entity_loaders);
    gui::draw_named_value(
      &state->gui_state, container, "n_valid_entity_loaders", debug_text
    );

    if (gui::draw_toggle(
      &state->gui_state, container, "Wireframe mode", &state->should_use_wireframe
    )) {
      state->should_use_wireframe = !state->should_use_wireframe;
      if (state->should_use_wireframe) {
        gui::set_heading(&state->gui_state, "Wireframe mode on.", 1.0f, 1.0f, 1.0f);
      } else {
        gui::set_heading(&state->gui_state, "Wireframe mode off.", 1.0f, 1.0f, 1.0f);
      }
    }

    if (gui::draw_toggle(
      &state->gui_state, container, "FPS limit", &state->should_limit_fps
    )) {
      state->should_limit_fps = !state->should_limit_fps;
      if (state->should_limit_fps) {
        gui::set_heading(&state->gui_state, "FPS limit enabled.", 1.0f, 1.0f, 1.0f);
      } else {
        gui::set_heading(&state->gui_state, "FPS limit disabled.", 1.0f, 1.0f, 1.0f);
      }
    }

    if (gui::draw_toggle(
      &state->gui_state, container, "Manual frame advance", &state->is_manual_frame_advance_enabled
    )) {
      state->is_manual_frame_advance_enabled = !state->is_manual_frame_advance_enabled;
      if (state->is_manual_frame_advance_enabled) {
        gui::set_heading(
          &state->gui_state, "Manual frame advance enabled.", 1.0f, 1.0f, 1.0f
        );
      } else {
        gui::set_heading(
          &state->gui_state, "Manual frame advance disabled.", 1.0f, 1.0f, 1.0f
        );
      }
    }

    if (gui::draw_toggle(
      &state->gui_state, container, "Pause", &state->should_pause
    )) {
      state->should_pause = !state->should_pause;
      if (state->should_pause) {
        gui::set_heading(&state->gui_state, "Pause enabled.", 1.0f, 1.0f, 1.0f);
      } else {
        gui::set_heading(&state->gui_state, "Pause disabled.", 1.0f, 1.0f, 1.0f);
      }
    }

    if (gui::draw_button(
      &state->gui_state, container, "Reload shaders"
    )) {
      materials::reload_shaders(&state->materials);
      gui::set_heading(&state->gui_state, "Shaders reloaded.", 1.0f, 1.0f, 1.0f);
    }

    if (gui::draw_button(
      &state->gui_state, container, "Delete PBO"
    )) {
      materials::delete_persistent_pbo(&state->persistent_pbo);
      gui::set_heading(&state->gui_state, "PBO deleted.", 1.0f, 1.0f, 1.0f);
    }
  }

  {
#if 1
    GuiContainer *container = gui::make_container(
      &state->gui_state, "Entities", v2(state->window_info.width - 400.0f, 25.0f)
    );
    get_scene_text_representation(debug_text, state);
    gui::draw_body_text(&state->gui_state, container, debug_text);
#endif
  }

  {
#if 1
    GuiContainer *container = gui::make_container(
      &state->gui_state, "Materials", v2(state->window_info.width - 600.0f, 25.0f)
    );
    get_materials_text_representation(debug_text, state);
    gui::draw_body_text(&state->gui_state, container, debug_text);
#endif
  }

  gui::draw_console(
    &state->gui_state, state->input_state.text_input
  );
  gui::render(&state->gui_state);
}
