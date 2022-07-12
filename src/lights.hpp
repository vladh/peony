// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "entities.hpp"
#include "spatial.hpp"

class lights {
public:
    // The position is used in positioning the shadow map, but not
    // in the light calculations. We need to scale the position
    // by some factor such that the shadow map covers the biggest
    // possible area.
    static constexpr v3 DIRECTIONAL_LIGHT_DISTANCE = v3(75.0f, 15.0f, 75.0f);

    enum class LightType { none, point, directional };

    struct Component {
        entities::Handle entity_handle;
        LightType type;
        v3 direction;
        v4 color;
        v4 attenuation;
    };

    struct ComponentSet {
        Array<Component> components;
    };

    struct State {
        real32 dir_light_angle;
    };

    static void adjust_dir_light_angle(f32 amount);
    static const char* light_type_to_string(LightType light_type);
    static LightType light_type_from_string(const char *str);
    static uint32 light_type_to_int(LightType light_type);
    static bool32 is_light_component_valid(Component *light_component);
    static void update_light_components(
        ComponentSet *light_component_set,
        spatial::ComponentSet *spatial_component_set,
        v3 camera_position
    );
    static void init(lights::State *state);

private:
    static lights::State *state;
};
