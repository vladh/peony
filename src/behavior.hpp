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
        EntityHandle entity_handle;
        Behavior behavior = Behavior::none;
    };

    struct ComponentSet {
        Array<Component> components;
    };

    struct State {
        ::State *state;
    };

    typedef void (*Function) (void *bevahior_state, EntityHandle entity_handle);

    static Function function_map[(uint32)Behavior::length];

    static char const * behavior_to_string(Behavior behavior);
    static Behavior behavior_from_string(const char *str);
    static bool32 is_behavior_component_valid(Component *behavior_component);
    static void update_behavior_components(EngineState *engine_state);
    static void init(
        behavior::State *behavior_state,
        ::State *state
    );

private:
    static behavior::State *state;
};
