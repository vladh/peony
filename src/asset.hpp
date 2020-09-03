#ifndef ASSET_H
#define ASSET_H

#include "memory.hpp"
#include "types.hpp"
#include "models.hpp"
#include "shader.hpp"


ShaderAsset* asset_get_shader_asset_by_name(State *state, const char *name);

#endif
