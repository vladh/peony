/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#include "debug_ui.hpp"
#include "entities.hpp"
#include "engine.hpp"
#include "intrinsics.hpp"


namespace debug_ui {
  pny_internal void get_entity_text_representation(
    char *text,
    EngineState *engine_state,
    Entity *entity,
    uint8 depth
  ) {
    EntityHandle handle = entity->handle;
    SpatialComponent *spatial_component =
      engine_state->spatial_component_set.components[handle];

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
      engine_state->drawable_component_set.components[handle]
    );
    bool32 has_light_component = lights::is_light_component_valid(
      engine_state->light_component_set.components[handle]
    );
    bool32 has_behavior_component = behavior::is_behavior_component_valid(
      engine_state->behavior_component_set.components[handle]
    );
    bool32 has_animation_component = anim::is_animation_component_valid(
      engine_state->animation_component_set.components[handle]
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
      each (child_spatial_component, engine_state->spatial_component_set.components) {
        if (
          child_spatial_component->parent_entity_handle ==
            spatial_component->entity_handle
        ) {
          n_children_found++;
          if (n_children_found > 5) {
            continue;
          }
          EntityHandle child_handle = child_spatial_component->entity_handle;
          Entity *child_entity = engine_state->entity_set.entities[child_handle];

          if (text[strlen(text) - 1] != '\n') {
            strcat(text, "\n");
          }
          get_entity_text_representation(text, engine_state, child_entity, depth + 1);
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


  pny_internal void get_scene_text_representation(
    char *text,
    EngineState *engine_state
  ) {
    text[0] = '\0';

    constexpr uint32 const MAX_N_SHOWN_ENTITIES = 35;
    uint32 idx_entity = 0;
    each (entity, engine_state->entity_set.entities) {
      if (idx_entity > MAX_N_SHOWN_ENTITIES) {
        sprintf(
          text + strlen(text),
          "...and %d more\n",
          engine_state->entity_set.entities.length - idx_entity
        );
        break;;
      }
      get_entity_text_representation(text, engine_state, entity, 0);
      idx_entity++;
    }

    if (text[strlen(text) - 1] == '\n') {
      text[strlen(text) - 1] = '\0';
    }
  }


  pny_internal void get_materials_text_representation(
    char *text,
    MaterialsState *materials_state,
    EngineState *engine_state
  ) {
    text[0] = '\0';

    strcat(text, "Internal:\n");

    uint32 idx = 0;
    each (material, materials_state->materials) {
      strcat(text, "- ");
      strcat(text, material->name);
      strcat(text, "\n");
      if (idx == engine_state->first_non_internal_material_idx - 1) {
        strcat(text, "Non-internal: \n");
      }
      idx++;
    }

    if (text[strlen(text) - 1] == '\n') {
      text[strlen(text) - 1] = '\0';
    }
  }
}


void debug_ui::render_debug_ui(
  EngineState *engine_state,
  RendererState *renderer_state,
  GuiState *gui_state,
  MaterialsState *materials_state,
  InputState *input_state,
  core::WindowSize *window_size
) {
  char debug_text[1 << 14];
  size_t dt_size = sizeof(debug_text);

  gui::start_drawing(gui_state);

  if (gui_state->heading_opacity > 0.0f) {
    gui::draw_heading(
      gui_state,
      gui_state->heading_text,
      v4(0.0f, 0.33f, 0.93f, gui_state->heading_opacity)
    );
    if (gui_state->heading_fadeout_delay > 0.0f) {
      gui_state->heading_fadeout_delay -= (real32)(*engine::g_dt);
    } else {
      gui_state->heading_opacity -=
        gui_state->heading_fadeout_duration * (real32)(*engine::g_dt);
    }
  }

  {
    strcpy(debug_text, "Peony debug info: ");
    strcat(debug_text, engine_state->current_scene_name);
    GuiContainer *container = gui::make_container(
      gui_state, debug_text, v2(25.0f, 25.0f)
    );

    snprintf(
      debug_text, dt_size, "%ux%u", window_size->width, window_size->height
    );
    gui::draw_named_value(gui_state, container, "screen size", debug_text);

    snprintf(
      debug_text, dt_size, "%ux%u",
      window_size->screencoord_width, window_size->screencoord_height
    );
    gui::draw_named_value(gui_state, container, "window size", debug_text);

    snprintf(debug_text, dt_size, "%u fps", engine_state->perf_counters.last_fps);
    gui::draw_named_value(gui_state, container, "fps", debug_text);

    snprintf(
      debug_text, dt_size, "%.2f ms", engine_state->perf_counters.dt_average * 1000.0f
    );
    gui::draw_named_value(gui_state, container, "dt", debug_text);

    snprintf(debug_text, dt_size, "%.2f", 1.0f + engine_state->timescale_diff);
    gui::draw_named_value(gui_state, container, "ts", debug_text);

    snprintf(debug_text, dt_size, engine_state->is_world_loaded ? "yes" : "no");
    gui::draw_named_value(gui_state, container, "is_world_loaded", debug_text);

    snprintf(debug_text, dt_size, "%u", materials_state->materials.length);
    gui::draw_named_value(
      gui_state, container, "materials.length", debug_text
    );

    snprintf(debug_text, dt_size, "%u", engine_state->entity_set.entities.length);
    gui::draw_named_value(gui_state, container, "entities.length", debug_text);

    snprintf(debug_text, dt_size, "%u", engine_state->model_loaders.length);
    gui::draw_named_value(
      gui_state, container, "model_loaders.length", debug_text
    );

    snprintf(debug_text, dt_size, "%u", engine_state->n_valid_model_loaders);
    gui::draw_named_value(
      gui_state, container, "n_valid_model_loaders", debug_text
    );

    snprintf(debug_text, dt_size, "%u", engine_state->entity_loader_set.loaders.length);
    gui::draw_named_value(
      gui_state, container, "entity_loader_set.length", debug_text
    );

    snprintf(debug_text, dt_size, "%u", engine_state->n_valid_entity_loaders);
    gui::draw_named_value(
      gui_state, container, "n_valid_entity_loaders", debug_text
    );

    if (gui::draw_toggle(
      gui_state, container, "Wireframe mode",
      &renderer_state->should_use_wireframe
    )) {
      renderer_state->should_use_wireframe =
        !renderer_state->should_use_wireframe;
      if (renderer_state->should_use_wireframe) {
        gui::set_heading(gui_state, "Wireframe mode on.", 1.0f, 1.0f, 1.0f);
      } else {
        gui::set_heading(gui_state, "Wireframe mode off.", 1.0f, 1.0f, 1.0f);
      }
    }

    if (gui::draw_toggle(
      gui_state, container, "FPS limit", &engine_state->should_limit_fps
    )) {
      engine_state->should_limit_fps = !engine_state->should_limit_fps;
      if (engine_state->should_limit_fps) {
        gui::set_heading(gui_state, "FPS limit enabled.", 1.0f, 1.0f, 1.0f);
      } else {
        gui::set_heading(gui_state, "FPS limit disabled.", 1.0f, 1.0f, 1.0f);
      }
    }

    if (gui::draw_toggle(
      gui_state, container, "Manual frame advance",
      &engine_state->is_manual_frame_advance_enabled
    )) {
      engine_state->is_manual_frame_advance_enabled =
        !engine_state->is_manual_frame_advance_enabled;
      if (engine_state->is_manual_frame_advance_enabled) {
        gui::set_heading(
          gui_state, "Manual frame advance enabled.", 1.0f, 1.0f, 1.0f
        );
      } else {
        gui::set_heading(
          gui_state, "Manual frame advance disabled.", 1.0f, 1.0f, 1.0f
        );
      }
    }

    if (gui::draw_toggle(
      gui_state, container, "Pause", &engine_state->should_pause
    )) {
      engine_state->should_pause = !engine_state->should_pause;
      if (engine_state->should_pause) {
        gui::set_heading(gui_state, "Pause enabled.", 1.0f, 1.0f, 1.0f);
      } else {
        gui::set_heading(gui_state, "Pause disabled.", 1.0f, 1.0f, 1.0f);
      }
    }

    if (gui::draw_button(
      gui_state, container, "Reload shaders"
    )) {
      materials::reload_shaders(&materials_state->materials);
      gui::set_heading(gui_state, "Shaders reloaded.", 1.0f, 1.0f, 1.0f);
    }

    if (gui::draw_button(
      gui_state, container, "Delete PBO"
    )) {
      materials::delete_persistent_pbo(&materials_state->persistent_pbo);
      gui::set_heading(gui_state, "PBO deleted.", 1.0f, 1.0f, 1.0f);
    }
  }

  {
    GuiContainer *container = gui::make_container(
      gui_state, "Entities", v2(window_size->width - 400.0f, 25.0f)
    );
    get_scene_text_representation(debug_text, engine_state);
    gui::draw_body_text(gui_state, container, debug_text);
  }

  {
    GuiContainer *container = gui::make_container(
      gui_state, "Materials", v2(window_size->width - 600.0f, 25.0f)
    );
    get_materials_text_representation(debug_text, materials_state, engine_state);
    gui::draw_body_text(gui_state, container, debug_text);
  }

  gui::draw_console(gui_state, input_state->text_input);
  gui::render(gui_state);
}
