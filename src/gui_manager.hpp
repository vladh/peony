class GuiManager {
public:
  Memory *memory;
  ShaderAsset *text_shader_asset;
  ShaderAsset *generic_shader_asset;
  Array<FontAsset> font_assets;
  uint32 vao;
  uint32 vbo;
  glm::mat4 text_projection;
  uint32 window_width;
  uint32 window_height;

  GuiManager(
    Memory *memory, Array<ShaderAsset> *shader_assets,
    uint32 window_width, uint32 window_height
  );
  void update_screen_dimensions(
    uint32 window_width, uint32 window_height
  );
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
  void draw_button(
    real32 x, real32 y, real32 w, real32 h,
    const char *text,
    real32 border_thickness,
    glm::vec4 color, glm::vec4 text_color, glm::vec4 border_color
  );
};
