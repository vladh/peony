// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "entities.hpp"
#include "behavior.hpp"

namespace behavior_functions {
  void test(
    BehaviorState *behavior_state,
    EntityHandle entity_handle
  );
  void char_movement_test(
    BehaviorState *behavior_state,
    EntityHandle entity_handle
  );
}
