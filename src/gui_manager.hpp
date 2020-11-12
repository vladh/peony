constexpr real32 LINE_HEIGHT_FACTOR = 1.5f;
constexpr uint32 N_MAX_CHARACTERS_PER_DRAW = 1024;
constexpr const char *DEFAULT_FONT = "resources/fonts/iosevka-regular.ttf";
constexpr uint32 DEFAULT_FONT_SIZE = 18;
constexpr uint32 GUI_VERTEX_LENGTH = 8;
constexpr size_t GUI_VERTEX_SIZE = sizeof(real32) * GUI_VERTEX_LENGTH;

constexpr glm::vec4 BUTTON_COLOR = glm::vec4(0.00f, 0.33f, 0.93f, 1.00f);
constexpr glm::vec4 BUTTON_BORDER_COLOR = glm::vec4(0.00f, 0.23f, 0.83f, 1.00f);
constexpr glm::vec4 BUTTON_HOVER_COLOR = glm::vec4(0.00f, 0.43f, 1.00f, 1.00f);
constexpr glm::vec4 BUTTON_ACTIVE_COLOR = glm::vec4(0.00f, 0.03f, 0.63f, 1.00f);
constexpr glm::vec4 BUTTON_TEXT_COLOR = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);

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
  uint32 window_width;
  uint32 window_height;

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
  void draw_text(
    const char* font_name, const char *str,
    real32 x, real32 y, real32 scale, glm::vec4 color
  );
  void draw_rect(
    real32 x, real32 y, real32 width, real32 height, glm::vec4 color
  );
  void draw_line(
    real32 start_x, real32 start_y, real32 end_x, real32 end_y,
    real32 thickness, glm::vec4 color
  );
  void draw_frame(
    real32 x0, real32 y0, real32 x1, real32 y1,
    real32 thickness, glm::vec4 color
  );
  bool32 draw_button(
    real32 x, real32 y, real32 w, real32 h,
    const char *text,
    real32 border_thickness
  );
};
