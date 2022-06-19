/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#pragma once

#include "types.hpp"
#include "entities.hpp"
#include "spatial.hpp"

namespace lights {
  // The position is used in positioning the shadow map, but not
  // in the light calculations. We need to scale the position
  // by some factor such that the shadow map covers the biggest
  // possible area.
  constexpr v3 DIRECTIONAL_LIGHT_DISTANCE = v3(75.0f, 15.0f, 75.0f);

  enum class LightType { none, point, directional };

  struct LightComponent {
    EntityHandle entity_handle;
    LightType type;
    v3 direction;
    v4 color;
    v4 attenuation;
  };

  struct LightComponentSet {
    Array<LightComponent> components;
  };

  struct LightsState {
    real32 dir_light_angle;
  };

  const char* light_type_to_string(LightType light_type);
  LightType light_type_from_string(const char *str);
  uint32 light_type_to_int(LightType light_type);
  bool32 is_light_component_valid(LightComponent *light_component);
  void update_light_components(
    LightsState *lights_state,
    LightComponentSet *light_component_set,
    SpatialComponentSet *spatial_component_set,
    v3 camera_position
  );
  void init(LightsState *state);
}

using lights::LightType, lights::LightComponent,
  lights::LightComponentSet, lights::LightsState;
