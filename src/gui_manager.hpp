constexpr real32 GUI_LINE_SPACING_FACTOR = 1.8f;
constexpr uint32 GUI_N_MAX_CHARACTERS_PER_DRAW = 1024;
constexpr const char *GUI_DEFAULT_FONT = "resources/fonts/SofiaProRegular.otf";
constexpr uint32 GUI_VERTEX_LENGTH = 8;
constexpr size_t GUI_VERTEX_SIZE = sizeof(real32) * GUI_VERTEX_LENGTH;
constexpr glm::vec4 GUI_BUTTON_COLOR = glm::vec4(0.00f, 0.33f, 0.93f, 1.00f);
constexpr glm::vec4 GUI_BUTTON_BORDER_COLOR = glm::vec4(0.00f, 0.23f, 0.83f, 1.00f);
constexpr glm::vec4 GUI_BUTTON_HOVER_COLOR = glm::vec4(0.00f, 0.43f, 1.00f, 1.00f);
constexpr glm::vec4 GUI_BUTTON_ACTIVE_COLOR = glm::vec4(0.00f, 0.03f, 0.63f, 1.00f);
constexpr glm::vec4 GUI_BUTTON_TEXT_COLOR = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
constexpr glm::vec2 GUI_BUTTON_AUTOSIZE_PADDING = glm::vec2(20.0f, 20.0f);
constexpr glm::vec2 GUI_BUTTON_DEFAULT_BORDER = glm::vec2(4.0f);

struct GuiContainer {
  const char *title;
  glm::vec2 position;
  // Dimensions include padding.
  glm::vec2 dimensions;
  glm::vec2 next_element_position;
  // The direction defines the main and orthogonal axes.
  // The main axis is the one elements are successively positioned on.
  // The main axis has a 1.0f, while the orthogonal axis has a 0.0f.
  glm::vec2 direction;
  glm::vec2 padding;
  uint32 n_elements;
  real32 element_margin;
};

class GuiManager {
public:
  Memory *memory;
  ShaderAsset *text_shader_asset;
  ShaderAsset *generic_shader_asset;
  InputManager *input_manager;
  GLFWcursor *requested_cursor;
  Array<FontAsset> font_assets;
  uint32 vao;
  uint32 vbo;
  glm::mat4 text_projection;
  glm::vec2 window_dimensions;

  GuiManager(
    Memory *memory, Array<ShaderAsset> *shader_assets,
    InputManager *input_manager,
    uint32 window_width, uint32 window_height
  );
  void update_screen_dimensions(
    uint32 window_width, uint32 window_height
  );
  void request_cursor(GLFWcursor *cursor);
  void set_cursor();
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
  GuiContainer make_container(
    const char *title, glm::vec2 position
  );
  void draw_container(GuiContainer *container);
  void draw_text(
    const char* font_name, const char *str,
    glm::vec2 position,
    glm::vec4 color
  );
  void draw_heading(
    const char* font_name, const char *str,
    glm::vec4 color
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
  bool32 draw_button(
    GuiContainer *container,
    const char *text
  );
};
