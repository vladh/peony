// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "mats.hpp"
#include "input.hpp"
#include "fonts.hpp"
#include "array.hpp"

namespace gui {
  constexpr uint32 MAX_CONSOLE_LINE_LENGTH = 200;
  constexpr uint32 MAX_N_CONSOLE_LINES = 30;
  constexpr char CONSOLE_SYMBOL[] = "> ";
  constexpr size_t CONSOLE_SYMBOL_LENGTH = 2;

  constexpr const char *MAIN_FONT_REGULAR = "SofiaProRegular.otf";
  constexpr const char *MAIN_FONT_BOLD = "SofiaProBold.otf";
  constexpr real32 LINE_SPACING_FACTOR = 1.8f;
  constexpr real32 CONSOLE_LINE_SPACING_FACTOR = 1.2f;

  constexpr uint32 MAX_N_VERTICES = 65536;
  constexpr uint32 VERTEX_LENGTH = 8;
  constexpr size_t VERTEX_SIZE = sizeof(real32) * VERTEX_LENGTH;

  constexpr v2 TEXT_SHADOW_OFFSET = v2(1.0f);

  constexpr v4 WINDOW_BG_COLOR = v4(0.20f, 0.20f, 0.20f, 1.00f);
  constexpr v4 CONSOLE_BG_COLOR = v4(0.10f, 0.10f, 0.10f, 0.95f);
  constexpr v4 MAIN_COLOR = v4(0.00f, 0.33f, 0.93f, 1.00f);
  constexpr v4 MAIN_DARKEN_COLOR = v4(0.00f, 0.23f, 0.83f, 1.00f);
  constexpr v4 MAIN_HOVER_COLOR = v4(0.00f, 0.43f, 1.00f, 1.00f);
  constexpr v4 MAIN_ACTIVE_COLOR = v4(0.00f, 0.13f, 0.73f, 1.00f);
  constexpr v4 LIGHT_TEXT_COLOR = v4(1.0f, 1.0f, 1.0f, 1.0f);
  constexpr v4 LIGHT_COLOR = v4(0.9f, 0.9f, 0.9f, 1.0f);
  constexpr v4 LIGHT_HOVER_COLOR = v4(1.0f, 1.0f, 1.0f, 1.0f);
  constexpr v4 LIGHT_ACTIVE_COLOR = v4(0.7f, 0.7f, 0.7f, 1.0f);
  constexpr v4 LIGHT_DARKEN_COLOR = v4(0.4f, 0.4f, 0.4f, 1.0f);

  constexpr v2 BUTTON_AUTOSIZE_PADDING = v2(20.0f, 20.0f);
  constexpr v2 BUTTON_DEFAULT_BORDER = v2(0.0f);

  constexpr real32 NAMED_VALUE_NAME_WIDTH = 250.0f;
  constexpr real32 TOGGLE_SPACING = 20.0f;
  constexpr v2 TOGGLE_BUTTON_SIZE = v2(20.0f);
  constexpr v2 TOGGLE_BUTTON_DEFAULT_BORDER = v2(0.0f);

  constexpr real32 MAX_CONSOLE_LOG_HEIGHT = 350.0f;
  constexpr v2 CONSOLE_PADDING = v2(10.0f);

  struct GuiContainer {
    const char *title;
    v2 position;
    // Dimensions include padding, title bar etc.
    v2 dimensions;
    v2 content_dimensions;
    v2 next_element_position;
    // The direction defines the main and orthogonal axes.
    // The main axis is the one elements are successively positioned on.
    // The main axis has a 1.0f, while the orthogonal axis has a 0.0f.
    v2 direction;
    v2 padding;
    real32 title_bar_height;
    uint32 n_elements;
    real32 element_margin;
  };

  struct GameConsole {
    bool32 is_enabled;
    char log[gui::MAX_N_CONSOLE_LINES][gui::MAX_CONSOLE_LINE_LENGTH];
    uint32 idx_log_start;
    uint32 idx_log_end;
  };

  struct GuiState {
    // NOTE: We're holding a pointer to another state here. What does this mean?
    InputState *input_state;

    shaders::Asset shader_asset;
    GLFWcursor *requested_cursor;
    Array<fonts::FontAsset> font_assets;
    mats::TextureAtlas texture_atlas;
    uint32 vao;
    uint32 vbo;
    v2 window_dimensions;
    uint32 n_vertices_pushed;

    // Console
    GameConsole game_console;

    // Containers
    Array<GuiContainer> containers;
    GuiContainer *container_being_moved;

    // Heading
    real32 heading_opacity;
    const char *heading_text;
    real32 heading_fadeout_duration;
    real32 heading_fadeout_delay;
  };

  extern GameConsole *g_console;

  void update_screen_dimensions(
    GuiState *gui_state,
    uint32 new_window_width, uint32 new_window_height
  );
  void update_mouse_button(GuiState *gui_state);
  void update_mouse(GuiState *gui_state);
  void start_drawing(GuiState *gui_state);
  void clear(GuiState *gui_state);
  void render(GuiState *gui_state);
  GuiContainer* make_container(
    GuiState *gui_state, const char *title, v2 position
  );
  void draw_heading(
    GuiState *gui_state,
    const char *str,
    v4 color
  );
  bool32 draw_toggle(
    GuiState *gui_state,
    GuiContainer *container,
    const char *text,
    bool32 *toggle_state
  );
  void draw_named_value(
    GuiState *gui_state,
    GuiContainer *container,
    const char *name_text,
    const char *value_text
  );
  void draw_body_text(
    GuiState *gui_state,
    GuiContainer *container,
    const char *text
  );
  bool32 draw_button(
    GuiState *gui_state,
    GuiContainer *container,
    const char *text
  );
  void draw_console(
    GuiState *gui_state,
    char *console_input_text
  );
  void log(const char *format, ...);
  void set_heading(
    GuiState *gui_state,
    const char *text, real32 opacity,
    real32 fadeout_duration, real32 fadeout_delay
  );
  void init(
    MemoryPool *memory_pool,
    GuiState* gui_state,
    InputState *input_state,
    uint32 window_width, uint32 window_height
  );
}

using gui::GuiContainer, gui::GameConsole, gui::GuiState;
