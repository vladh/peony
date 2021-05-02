#ifndef GUI_HPP
#define GUI_HPP

constexpr uint32 GUI_MAX_CONSOLE_LINE_LENGTH = 200;
constexpr uint32 GUI_MAX_N_CONSOLE_LINES = 30;
constexpr char GUI_CONSOLE_SYMBOL[] = "> ";
constexpr size_t GUI_CONSOLE_SYMBOL_LENGTH = 2;

struct GuiContainer {
  const char *title;
  glm::vec2 position;
  // Dimensions include padding, title bar etc.
  glm::vec2 dimensions;
  glm::vec2 content_dimensions;
  glm::vec2 next_element_position;
  // The direction defines the main and orthogonal axes.
  // The main axis is the one elements are successively positioned on.
  // The main axis has a 1.0f, while the orthogonal axis has a 0.0f.
  glm::vec2 direction;
  glm::vec2 padding;
  real32 title_bar_height;
  uint32 n_elements;
  real32 element_margin;
};

struct GuiState {
  ShaderAsset shader_asset;
  InputState *input_state;
  GLFWcursor *requested_cursor;
  Array<FontAsset> font_assets;
  TextureAtlas texture_atlas;
  uint32 vao;
  uint32 vbo;
  glm::vec2 window_dimensions;
  uint32 n_vertices_pushed;

  // Containers
  Array<GuiContainer> containers;
  GuiContainer *container_being_moved;

  // Heading
  real32 heading_opacity;
  const char *heading_text;
  real32 heading_fadeout_duration;
  real32 heading_fadeout_delay;

  // Console
  bool32 is_console_enabled;
  char console_log[GUI_MAX_N_CONSOLE_LINES][GUI_MAX_CONSOLE_LINE_LENGTH];
  uint32 idx_console_log_start;
  uint32 idx_console_log_end;
};

namespace Gui {
  constexpr const char *GUI_MAIN_FONT_REGULAR = "SofiaProRegular.otf";
  constexpr const char *GUI_MAIN_FONT_BOLD = "SofiaProBold.otf";
  constexpr real32 GUI_LINE_SPACING_FACTOR = 1.8f;
  constexpr real32 GUI_CONSOLE_LINE_SPACING_FACTOR = 1.2f;

  constexpr uint32 GUI_MAX_N_VERTICES = 16384;
  constexpr uint32 GUI_VERTEX_LENGTH = 8;
  constexpr size_t GUI_VERTEX_SIZE = sizeof(real32) * GUI_VERTEX_LENGTH;

  constexpr glm::vec2 GUI_TEXT_SHADOW_OFFSET = glm::vec2(1.0f);

  constexpr glm::vec4 GUI_WINDOW_BG_COLOR = glm::vec4(0.20f, 0.20f, 0.20f, 1.00f);
  constexpr glm::vec4 GUI_CONSOLE_BG_COLOR = glm::vec4(0.10f, 0.10f, 0.10f, 0.95f);
  constexpr glm::vec4 GUI_MAIN_COLOR = glm::vec4(0.00f, 0.33f, 0.93f, 1.00f);
  constexpr glm::vec4 GUI_MAIN_DARKEN_COLOR = glm::vec4(0.00f, 0.23f, 0.83f, 1.00f);
  constexpr glm::vec4 GUI_MAIN_HOVER_COLOR = glm::vec4(0.00f, 0.43f, 1.00f, 1.00f);
  constexpr glm::vec4 GUI_MAIN_ACTIVE_COLOR = glm::vec4(0.00f, 0.13f, 0.73f, 1.00f);
  constexpr glm::vec4 GUI_LIGHT_TEXT_COLOR = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  constexpr glm::vec4 GUI_LIGHT_COLOR = glm::vec4(0.9f, 0.9f, 0.9f, 1.0f);
  constexpr glm::vec4 GUI_LIGHT_HOVER_COLOR = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
  constexpr glm::vec4 GUI_LIGHT_ACTIVE_COLOR = glm::vec4(0.7f, 0.7f, 0.7f, 1.0f);
  constexpr glm::vec4 GUI_LIGHT_DARKEN_COLOR = glm::vec4(0.4f, 0.4f, 0.4f, 1.0f);

  constexpr glm::vec2 GUI_BUTTON_AUTOSIZE_PADDING = glm::vec2(20.0f, 20.0f);
  constexpr glm::vec2 GUI_BUTTON_DEFAULT_BORDER = glm::vec2(0.0f);

  constexpr real32 GUI_NAMED_VALUE_NAME_WIDTH = 250.0f;
  constexpr real32 GUI_TOGGLE_SPACING = 20.0f;
  constexpr glm::vec2 GUI_TOGGLE_BUTTON_SIZE = glm::vec2(20.0f);
  constexpr glm::vec2 GUI_TOGGLE_BUTTON_DEFAULT_BORDER = glm::vec2(0.0f);

  constexpr real32 GUI_MAX_CONSOLE_LOG_HEIGHT = 350.0f;
  constexpr glm::vec2 GUI_CONSOLE_PADDING = glm::vec2(10.0f);

  void update_screen_dimensions(
    GuiState *gui_state, uint32 window_width, uint32 window_height
  );
  void update_mouse_button(GuiState *gui_state);
  void update_mouse(GuiState *gui_state);
  void request_cursor(GuiState *gui_state, GLFWcursor *cursor);
  void set_cursor(GuiState *gui_state);
  void start_drawing(GuiState *gui_state);
  void push_vertices(GuiState *gui_state, real32 *vertices, uint32 n_vertices);
  void render(GuiState *gui_state);
  glm::vec2 get_text_dimensions(FontAsset *font_asset, const char *str);
  glm::vec2 center_bb(
    glm::vec2 container_position,
    glm::vec2 container_dimensions,
    glm::vec2 element_dimensions
  );
  glm::vec2 add_element_to_container(
    GuiContainer *container, glm::vec2 element_dimensions
  );
  void draw_container(GuiState *gui_state, GuiContainer *container);
  GuiContainer* make_container(
    GuiState *gui_state, const char *title, glm::vec2 position
  );
  void draw_text(
    GuiState *gui_state,
    const char* font_name, const char *str,
    glm::vec2 position,
    glm::vec4 color
  );
  void draw_text_shadow(
    GuiState *gui_state,
    const char* font_name, const char *str,
    glm::vec2 position,
    glm::vec4 color
  );
  void draw_heading(
    GuiState *gui_state,
    const char *str,
    glm::vec4 color
  );
  void draw_rect(
    GuiState *gui_state,
    glm::vec2 position,
    glm::vec2 dimensions,
    glm::vec4 color
  );
  void draw_line(
    GuiState *gui_state,
    glm::vec2 start, glm::vec2 end,
    real32 thickness, glm::vec4 color
  );
  void draw_frame(
    GuiState *gui_state,
    glm::vec2 position, glm::vec2 bottomright,
    glm::vec2 thickness, glm::vec4 color
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
  void console_print(
    GuiState *gui_state,
    const char *text
  );
  GuiState* init_gui_state(
    GuiState *gui_state,
    MemoryPool *memory_pool,
    InputState *input_state,
    uint32 window_width, uint32 window_height
  );
}

#endif
