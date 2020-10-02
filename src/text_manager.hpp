class TextManager {
public:
  ShaderAsset* shader_asset;
  Array<FontAsset> font_assets;
  uint32 vao;
  uint32 vbo;

  TextManager(
    Memory *memory, ShaderAsset *shader_asset,
    uint32 window_width, uint32 window_height
  );
  void update_text_projection(
    uint32 window_width, uint32 window_height
  );
  void draw(
    const char* font_name, const char *str,
    real32 x, real32 y, real32 scale, glm::vec4 color
  );
};
