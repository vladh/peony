// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "debug.hpp"
#include "util.hpp"
#include "logs.hpp"
#include "models.hpp"


bool32
drawable::is_component_valid(drawable::Component *drawable_component)
{
    return models::is_mesh_valid(&drawable_component->mesh);
}


void
drawable::destroy_component(drawable::Component *drawable_component)
{
    if (!is_component_valid(drawable_component)) {
        return;
    }
    models::destroy_mesh(&drawable_component->mesh);
}

