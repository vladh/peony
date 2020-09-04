#ifndef ASSET_H
#define ASSET_H

#include "types.hpp"
#include "shader.hpp"
#include "models.hpp"


typedef struct AssetInfo {
  const char *name;
} AssetInfo;

typedef struct ModelAsset {
  AssetInfo info;
  Model model;
} ModelAsset;

typedef struct ShaderAsset {
  AssetInfo info;
  Shader shader;
} ShaderAsset;

ShaderAsset* asset_get_shader_asset_by_name(
  ShaderAsset *shader_assets, uint32 n_shader_assets, const char *name
);
ModelAsset* asset_get_model_asset_by_name(
  ModelAsset *model_assets[], uint32 n_model_assets, const char *name
);

#endif
