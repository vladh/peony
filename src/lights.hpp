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

  enum class LightType {none, point, directional};

  struct LightComponent {
    EntityHandle entity_handle;
    LightType type = LightType::none;
    v3 direction = v3(0.0f);
    v4 color = v4(0.0f);
    v4 attenuation = v4(0.0f);
  };

  struct LightComponentSet {
    Array<LightComponent> components;
  };

  const char* light_type_to_string(LightType light_type);
  LightType light_type_from_string(const char *str);
  uint32 light_type_to_int(LightType light_type);
  bool32 is_light_component_valid(LightComponent *light_component);
  void update_light_components(
    LightComponentSet *light_component_set,
    SpatialComponentSet *spatial_component_set,
    real64 t,
    v3 camera_position,
    real32 dir_light_angle
  );
}

using lights::LightType, lights::LightComponent,
  lights::LightComponentSet;
