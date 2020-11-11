class TextManager {
public:
  ShaderAsset* shader_asset;
  Array<FontAsset> font_assets;
  uint32 vao;
  uint32 vbo;
  glm::mat4 text_projection;

  TextManager(
    Memory *memory, ShaderAsset *shader_asset
  );
  void draw(
    const char* font_name, const char *str,
    real32 x, real32 y, real32 scale, glm::vec4 color
  );
};
