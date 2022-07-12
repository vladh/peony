// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "logs.hpp"
#include "lights.hpp"
#include "engine.hpp"
#include "intrinsics.hpp"


lights::State *lights::state = nullptr;


void
lights::adjust_dir_light_angle(f32 amount)
{
    lights::state->dir_light_angle += amount;
}


char const *
lights::light_type_to_string(LightType light_type)
{
    if (light_type == LightType::none) {
        return "none";
    } else if (light_type == LightType::point) {
        return "point";
    } else if (light_type == LightType::directional) {
        return "directional";
    } else {
        logs::error("Don't know how to convert LightType to string: %d", light_type);
        return "<unknown>";
    }
}


lights::LightType
lights::light_type_from_string(char const *str)
{
    if (strcmp(str, "none") == 0) {
        return LightType::none;
    } else if (strcmp(str, "point") == 0) {
        return LightType::point;
    } else if (strcmp(str, "directional") == 0) {
        return LightType::directional;
    } else {
        logs::fatal("Could not parse LightType: %s", str);
        return LightType::none;
    }
}


u32
lights::light_type_to_int(LightType light_type)
{
    if (light_type == LightType::point) {
        return 1;
    } else if (light_type == LightType::directional) {
        return 2;
    }
    return 0;
}


bool
lights::is_light_component_valid(lights::Component *light_component)
{
    return light_component->type != LightType::none;
}


void
lights::update_light_components(
    lights::ComponentSet *light_component_set,
    spatial::ComponentSet *spatial_component_set,
    v3 camera_position
) {
    each (light_component, light_component_set->components) {
        if (light_component->entity_handle == entities::NO_ENTITY_HANDLE) {
            continue;
        }

        spatial::Component *spatial_component =
            spatial_component_set->components[light_component->entity_handle];

        if (!(
            is_light_component_valid(light_component) &&
            spatial::is_spatial_component_valid(spatial_component)
        )) {
            continue;
        }

        if (light_component->type == LightType::point) {
            light_component->color.b = ((f32)sin(engine::get_t()) + 1.0f) / 2.0f * 50.0f;
        }

        // For the sun! :)
        if (light_component->type == LightType::directional) {
            spatial_component->position = camera_position +
                -light_component->direction * DIRECTIONAL_LIGHT_DISTANCE;
            light_component->direction = v3(sin(lights::state->dir_light_angle),
                -cos(lights::state->dir_light_angle), 0.0f);
        }
    }
}


void
lights::init(lights::State *lights_state)
{
    lights::state = lights_state;
    lights::state->dir_light_angle = radians(55.0f);
}
