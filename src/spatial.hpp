// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "entities.hpp"

class spatial {
public:
    struct Obb {
        v3 center;
        v3 x_axis;
        v3 y_axis; // We can get the z axis with a cross product
        v3 extents;
    };

    struct Face {
        v3 vertices[4];
    };

    struct Ray {
        v3 origin;
        v3 direction;
    };


    struct Component {
        entities::Handle entity_handle;
        v3 position;
        quat rotation;
        v3 scale;
        entities::Handle parent_entity_handle;
    };


    struct ModelMatrixCache {
        m4 last_model_matrix;
        Component *last_model_matrix_spatial_component;
    };

    struct ComponentSet {
        Array<Component> components;
    };

    struct State {
        ComponentSet component_set;
    };


    static void print_spatial_component(Component *spatial_component);
    static bool does_spatial_component_have_dimensions(Component *spatial_component);
    static bool is_spatial_component_valid(Component *spatial_component);
    static m4 make_model_matrix(
        Component *spatial_component,
        ModelMatrixCache *cache
    );
    static Array<spatial::Component> * get_components();
    static spatial::Component * get_component(entities::Handle entity_handle);
    static void init(spatial::State *spatial_state, memory::Pool *asset_memory_pool);

private:
    static spatial::State *state;
};
