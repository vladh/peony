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

    struct ComponentSet {
        Array<Component> components;
    };

    struct State {
        ::State *state;
    };

    typedef void (*Function) (entities::Handle entity_handle);

    static Function function_map[(u32)Behavior::length];

    static char const * behavior_to_string(Behavior behavior);
    static Behavior behavior_from_string(const char *str);
    static bool is_behavior_component_valid(Component *behavior_component);
    static void update_behavior_components(ComponentSet *component_set);
    static void init(
        behavior::State *behavior_state,
        ::State *state
    );

private:
    static behavior::State *state;
};
