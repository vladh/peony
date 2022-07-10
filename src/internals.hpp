// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "engine.hpp"
#include "renderer.hpp"
#include "mats.hpp"

namespace internals {
  void create_internal_materials(
    EngineState *engine_state,
    renderer::State *renderer_state,
    mats::State *materials_state
  );
  void create_internal_entities(
    EngineState *engine_state,
    renderer::State *renderer_state,
    mats::State *materials_state
  );
  void init(
    EngineState *engine_state,
    renderer::State *renderer_state,
    mats::State *materials_state
  );
}
