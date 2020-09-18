ShaderAsset* asset_get_shader_asset_by_name(
  Array<ShaderAsset> *shader_assets, const char *name
) {
  for (uint32 idx = 0; idx < shader_assets->size; idx++) {
    ShaderAsset *asset = shader_assets->items + idx;
    if (strcmp(asset->info.name, name) == 0) {
      return asset;
    }
  }
  log_warning("Could not find ShaderAsset with name %s", name);
  return nullptr;
}

FontAsset* asset_get_font_asset_by_name(
  Array<FontAsset> *font_assets, const char *name
) {
  for (uint32 idx = 0; idx < font_assets->size; idx++) {
    FontAsset *asset = font_assets->items + idx;
    if (strcmp(asset->info.name, name) == 0) {
      return asset;
    }
  }
  log_warning("Could not find FontAsset with name %s", name);
  return nullptr;
}

ModelAsset* asset_get_model_asset_by_name(
  Array<ModelAsset*> *model_assets, const char *name
) {
  for (uint32 idx = 0; idx < model_assets->size; idx++) {
    ModelAsset *asset = *(model_assets->items + idx);
    if (strcmp(asset->info.name, name) == 0) {
      return asset;
    }
  }
  log_warning("Could not find ModelAsset with name %s", name);
  return nullptr;
}
