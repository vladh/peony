#ifndef ASSET_H
#define ASSET_H

#include "types.hpp"
#include "shader.hpp"
#include "models.hpp"
#include "array.hpp"


struct Model;

struct AssetInfo {
  const char *name;
};

struct ModelAsset {
  AssetInfo info;
  Model model;
};

struct ShaderAsset {
  AssetInfo info;
  Shader shader;
};

ShaderAsset* asset_get_shader_asset_by_name(
  Array<ShaderAsset> *shader_assets, const char *name
);
ModelAsset* asset_get_model_asset_by_name(
  ModelAsset *model_assets[], uint32 n_model_assets, const char *name
);

#endif
