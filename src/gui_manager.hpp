constexpr const char *GUI_MAIN_FONT_REGULAR = "resources/fonts/SofiaProRegular.otf";
constexpr const char *GUI_MAIN_FONT_BOLD = "resources/fonts/SofiaProBold.otf";
constexpr real32 GUI_LINE_SPACING_FACTOR = 1.8f;

constexpr uint32 GUI_N_MAX_VERTICES = 16384;
constexpr uint32 GUI_VERTEX_LENGTH = 8;
constexpr size_t GUI_VERTEX_SIZE = sizeof(real32) * GUI_VERTEX_LENGTH;

constexpr glm::vec2 GUI_TEXT_SHADOW_OFFSET = glm::vec2(1.0f);

constexpr glm::vec4 GUI_WINDOW_BG_COLOR = glm::vec4(0.20f, 0.20f, 0.20f, 1.00f);
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

constexpr real32 GUI_NAMED_VALUE_NAME_WIDTH = 200.0f;
constexpr real32 GUI_TOGGLE_SPACING = 20.0f;
constexpr glm::vec2 GUI_TOGGLE_BUTTON_SIZE = glm::vec2(20.0f);
constexpr glm::vec2 GUI_TOGGLE_BUTTON_DEFAULT_BORDER = glm::vec2(0.0f);


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


class GuiManager {
public:
  Memory *memory;
  ShaderAsset *shader_asset;
  InputManager *input_manager;
  GLFWcursor *requested_cursor;
  Array<FontAsset> font_assets;
  TextureAtlas texture_atlas;
  Array<GuiContainer> containers;
  GuiContainer *container_being_moved;
  uint32 vao;
  uint32 vbo;
  glm::mat4 text_projection;
  glm::vec2 window_dimensions;
  uint32 n_vertices_pushed;

  GuiManager(
    Memory *memory, Array<ShaderAsset> *shader_assets,
    InputManager *input_manager,
    uint32 window_width, uint32 window_height
  );
  void update_screen_dimensions(
    uint32 window_width, uint32 window_height
  );
  void update_mouse_button();
  void update_mouse();
  void request_cursor(GLFWcursor *cursor);
  void set_cursor();
  void start_drawing();
  void push_vertices(real32 *vertices, uint32 n_vertices);
  void render();
  glm::vec2 get_text_dimensions(
    const char* font_name, const char *str
  );
  glm::vec2 center_bb(
    glm::vec2 container_position,
    glm::vec2 container_dimensions,
    glm::vec2 element_dimensions
  );
  glm::vec2 add_element_to_container(
    GuiContainer *container, glm::vec2 element_dimensions
  );
  void draw_container(GuiContainer *container);
  GuiContainer* make_container(
    const char *title, glm::vec2 position
  );
  void draw_text(
    const char* font_name, const char *str,
    glm::vec2 position,
    glm::vec4 color
  );
  void draw_text_shadow(
    const char* font_name, const char *str,
    glm::vec2 position,
    glm::vec4 color
  );
  void draw_heading(
    const char *str, glm::vec4 color
  );
  void draw_rect(
    glm::vec2 position, glm::vec2 dimensions, glm::vec4 color
  );
  void draw_line(
    glm::vec2 start, glm::vec2 end,
    real32 thickness, glm::vec4 color
  );
  void draw_frame(
    glm::vec2 position, glm::vec2 bottomright,
    glm::vec2 thickness, glm::vec4 color
  );
  bool32 draw_toggle(
    GuiContainer *container,
    const char *text,
    bool32 *toggle_state
  );
  void draw_named_value(
    GuiContainer *container,
    const char *name_text,
    const char *value_text
  );
  void draw_body_text(
    GuiContainer *container,
    const char *text
  );
  bool32 draw_button(
    GuiContainer *container,
    const char *text
  );
};
