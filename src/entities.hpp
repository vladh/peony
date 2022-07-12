// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "constants.hpp"
#include "array.hpp"

namespace entities {
  // NOTE: 0 is an invalid handle.
  typedef uint32 EntityHandle;

  struct Entity {
    EntityHandle handle;
    char debug_name[MAX_DEBUG_NAME_LENGTH];
  };

  struct EntitySet {
    Array<Entity> entities;
    // The handle of the next entity which has not yet been created.
    // NOTE: 0 is an invalid handle.
    EntityHandle next_handle;
    // Certain entities at the start of our set are internal.
    // Remember the handle after we're done creating the internal entities,
    // so we can iterate through the non-internal ones, if we so desire.
    // This assumes all our internal entities will be contiguous and at the
    // start of our set.
    EntityHandle first_non_internal_handle;
  };

  constexpr EntityHandle NO_ENTITY_HANDLE = 0;

  EntityHandle make_handle();
  Entity* add_entity_to_set(const char *debug_name);
}

using entities::EntityHandle, entities::Entity, entities::EntitySet;
