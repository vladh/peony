#ifndef ASSET_H
#define ASSET_H

struct Model;
struct Font;

struct AssetInfo {
  const char *name;
};

struct ModelAsset {
  AssetInfo info;
  Model model;
};

struct FontAsset {
  AssetInfo info;
  Font font;
};

struct ShaderAsset {
  AssetInfo info;
  Shader shader;
};

ShaderAsset* asset_get_shader_asset_by_name(
  Array<ShaderAsset> *shader_assets, const char *name
);
FontAsset* asset_get_font_asset_by_name(
  Array<FontAsset> *font_assets, const char *name
);
ModelAsset* asset_get_model_asset_by_name(
  Array<ModelAsset*> *model_assets, const char *name
);

#endif
