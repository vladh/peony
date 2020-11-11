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
};
