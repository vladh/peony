// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "logs.hpp"
#include "behavior_functions.hpp"
#include "behavior.hpp"
#include "engine.hpp"
#include "intrinsics.hpp"


behavior::State *behavior::state = nullptr;


behavior::Function behavior::function_map[(uint32)Behavior::length] = {
    (behavior::Function)nullptr,
    (behavior::Function)behavior_functions::test,
    (behavior::Function)behavior_functions::char_movement_test,
};


char const *
behavior::behavior_to_string(Behavior behavior)
{
    if (behavior == Behavior::none) {
        return "none";
    } else if (behavior == Behavior::test) {
        return "test";
    } else if (behavior == Behavior::char_movement_test) {
        return "char_movement_test";
    } else {
        logs::error("Don't know how to convert Behavior to string: %d", behavior);
        return "<unknown>";
    }
}


behavior::Behavior
behavior::behavior_from_string(char const *str)
{
    if (strcmp(str, "none") == 0) {
        return Behavior::none;
    } else if (strcmp(str, "test") == 0) {
        return Behavior::test;
    } else if (strcmp(str, "char_movement_test") == 0) {
        return Behavior::char_movement_test;
    } else {
        logs::fatal("Could not parse Behavior: %s", str);
        return Behavior::none;
    }
}


bool32
behavior::is_behavior_component_valid(behavior::Component *behavior_component)
{
    return behavior_component->behavior != Behavior::none;
}


void
behavior::update_behavior_components(
    EngineState *engine_state
) {
    each (behavior_component, engine_state->behavior_component_set.components) {
        if (!is_behavior_component_valid(behavior_component)) {
            continue;
        }

        EntityHandle entity_handle = behavior_component->entity_handle;

        auto handler = function_map[(uint32)behavior_component->behavior];
        if (handler) {
            handler(behavior::state, entity_handle);
        }
    }
}


void behavior::init(
    behavior::State *behavior_state,
    ::State *state
) {
    behavior::state = behavior_state;
    // NOTE: behavior needs the global state to pass to the behavior functions
    behavior::state->state = state;
}
