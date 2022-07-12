// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "entities.hpp"
#include "behavior.hpp"

class behavior_functions {
public:
    static void test(
        behavior::State *behavior_state,
        EntityHandle entity_handle
    );
    static void char_movement_test(
        behavior::State *behavior_state,
        EntityHandle entity_handle
    );
};
