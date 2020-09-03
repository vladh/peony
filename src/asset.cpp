#include "memory.hpp"
#include "log.hpp"
#include "asset.hpp"


ShaderAsset* asset_get_shader_asset_by_name(State *state, const char *name) {
  /* log_info("Getting shader asset: %s", name); */
  for (uint32 idx = 0; idx < state->n_shader_assets; idx++) {
    ShaderAsset *asset = state->shader_assets + idx;
    if (strcmp(asset->info.name, name) == 0) {
      return asset;
    }
  }
  return nullptr;
}
