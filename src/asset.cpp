#include "memory.hpp"
#include "log.hpp"
#include "asset.hpp"


ShaderAsset* asset_get_shader_asset_by_name(
  Array<ShaderAsset> *shader_assets, const char *name
) {
  for (uint32 idx = 0; idx < shader_assets->size; idx++) {
    ShaderAsset *asset = shader_assets->items + idx;
    if (strcmp(asset->info.name, name) == 0) {
      return asset;
    }
  }
  return nullptr;
}

ModelAsset* asset_get_model_asset_by_name(
  ModelAsset *model_assets[], uint32 n_model_assets, const char *name
) {
  for (uint32 idx = 0; idx < n_model_assets; idx++) {
    ModelAsset *asset = model_assets[idx];
    if (strcmp(asset->info.name, name) == 0) {
      return asset;
    }
  }
  return nullptr;
}
