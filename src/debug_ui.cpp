// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

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
    bool32 has_drawable_component = drawable::is_component_valid(
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
    mats::State *materials_state,
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
  renderer::State *renderer_state,
  mats::State *materials_state,
  InputState *input_state,
  renderer::WindowSize *window_size
) {
  char debug_text[1 << 14];
  size_t dt_size = sizeof(debug_text);

  renderer::start_drawing_gui();

  gui::tick_heading();

  {
    strcpy(debug_text, "Peony debug info: ");
    strcat(debug_text, engine_state->current_scene_name);
    gui::Container *container = gui::make_container(
      debug_text, v2(25.0f, 25.0f)
    );

    snprintf(
      debug_text, dt_size, "%ux%u", window_size->width, window_size->height
    );
    gui::draw_named_value(container, "screen size", debug_text);

    snprintf(
      debug_text, dt_size, "%ux%u",
      window_size->screencoord_width, window_size->screencoord_height
    );
    gui::draw_named_value(container, "window size", debug_text);

    snprintf(debug_text, dt_size, "%u fps", engine_state->perf_counters.last_fps);
    gui::draw_named_value(container, "fps", debug_text);

    snprintf(
      debug_text, dt_size, "%.2f ms", engine_state->perf_counters.dt_average * 1000.0f
    );
    gui::draw_named_value(container, "dt", debug_text);

    snprintf(debug_text, dt_size, "%.2f", 1.0f + engine_state->timescale_diff);
    gui::draw_named_value(container, "ts", debug_text);

    snprintf(debug_text, dt_size, engine_state->is_world_loaded ? "yes" : "no");
    gui::draw_named_value(container, "is_world_loaded", debug_text);

    snprintf(debug_text, dt_size, "%u", materials_state->materials.length);
    gui::draw_named_value(
      container, "materials.length", debug_text
    );

    snprintf(debug_text, dt_size, "%u", engine_state->entity_set.entities.length);
    gui::draw_named_value(container, "entities.length", debug_text);

    snprintf(debug_text, dt_size, "%u", engine_state->model_loaders.length);
    gui::draw_named_value(
      container, "model_loaders.length", debug_text
    );

    snprintf(debug_text, dt_size, "%u", engine_state->n_valid_model_loaders);
    gui::draw_named_value(
      container, "n_valid_model_loaders", debug_text
    );

    snprintf(debug_text, dt_size, "%u", engine_state->entity_loader_set.loaders.length);
    gui::draw_named_value(
      container, "entity_loader_set.length", debug_text
    );

    snprintf(debug_text, dt_size, "%u", engine_state->n_valid_entity_loaders);
    gui::draw_named_value(
      container, "n_valid_entity_loaders", debug_text
    );

    if (gui::draw_toggle(
      container, "Wireframe mode",
      &renderer_state->should_use_wireframe
    )) {
      renderer_state->should_use_wireframe =
        !renderer_state->should_use_wireframe;
      if (renderer_state->should_use_wireframe) {
        gui::set_heading("Wireframe mode on.", 1.0f, 1.0f, 1.0f);
      } else {
        gui::set_heading("Wireframe mode off.", 1.0f, 1.0f, 1.0f);
      }
    }

    if (gui::draw_toggle(
      container, "FPS limit", &engine_state->should_limit_fps
    )) {
      engine_state->should_limit_fps = !engine_state->should_limit_fps;
      if (engine_state->should_limit_fps) {
        gui::set_heading("FPS limit enabled.", 1.0f, 1.0f, 1.0f);
      } else {
        gui::set_heading("FPS limit disabled.", 1.0f, 1.0f, 1.0f);
      }
    }

    if (gui::draw_toggle(
      container, "Manual frame advance",
      &engine_state->is_manual_frame_advance_enabled
    )) {
      engine_state->is_manual_frame_advance_enabled =
        !engine_state->is_manual_frame_advance_enabled;
      if (engine_state->is_manual_frame_advance_enabled) {
        gui::set_heading(
          "Manual frame advance enabled.", 1.0f, 1.0f, 1.0f
        );
      } else {
        gui::set_heading(
          "Manual frame advance disabled.", 1.0f, 1.0f, 1.0f
        );
      }
    }

    if (gui::draw_toggle(
      container, "Pause", &engine_state->should_pause
    )) {
      engine_state->should_pause = !engine_state->should_pause;
      if (engine_state->should_pause) {
        gui::set_heading("Pause enabled.", 1.0f, 1.0f, 1.0f);
      } else {
        gui::set_heading("Pause disabled.", 1.0f, 1.0f, 1.0f);
      }
    }

    if (gui::draw_button(
      container, "Reload shaders"
    )) {
      mats::reload_shaders(&materials_state->materials);
      gui::set_heading("Shaders reloaded.", 1.0f, 1.0f, 1.0f);
    }

    if (gui::draw_button(
      container, "Delete PBO"
    )) {
      mats::delete_persistent_pbo(&materials_state->persistent_pbo);
      gui::set_heading("PBO deleted.", 1.0f, 1.0f, 1.0f);
    }
  }

  {
    gui::Container *container = gui::make_container(
      "Entities", v2(window_size->width - 400.0f, 25.0f)
    );
    get_scene_text_representation(debug_text, engine_state);
    gui::draw_body_text(container, debug_text);
  }

  {
    gui::Container *container = gui::make_container(
      "Materials", v2(window_size->width - 600.0f, 25.0f)
    );
    get_materials_text_representation(debug_text, materials_state, engine_state);
    gui::draw_body_text(container, debug_text);
  }

  gui::draw_console(input_state->text_input);
  renderer::render_gui();
  gui::update();
}
