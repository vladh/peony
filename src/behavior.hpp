// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "array.hpp"
#include "entities.hpp"
#include "spatial.hpp"

struct State;
struct EngineState;

class behavior {
public:
    enum class Behavior {
        none,
        test,
        char_movement_test,
        length
    };

    struct Component {
        entities::Handle entity_handle;
        Behavior behavior = Behavior::none;
    };

    struct State {
        Array<Component> components;
        ::State *state;
    };

    typedef void (*Function) (entities::Handle entity_handle);

    static Function function_map[(u32)Behavior::length];

    static char const * behavior_to_string(Behavior behavior);
    static Behavior behavior_from_string(const char *str);
    static bool is_behavior_component_valid(Component *behavior_component);
    static void update_behavior_components();
    static Array<behavior::Component> * get_components();
    static behavior::Component * get_component(entities::Handle entity_handle);
    static void init(
        behavior::State *behavior_state,
        memory::Pool *asset_memory_pool,
        ::State *state
    );

private:
    static behavior::State *state;
};
