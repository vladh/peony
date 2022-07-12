// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"
#include "constants.hpp"
#include "array.hpp"

class entities {
public:
    // NOTE: 0 is an invalid handle.
    typedef uint32 Handle;

    struct Entity {
        Handle handle;
        char debug_name[MAX_DEBUG_NAME_LENGTH];
    };

    struct Set {
        Array<Entity> entities;
        // The handle of the next entity which has not yet been created.
        // NOTE: 0 is an invalid handle.
        Handle next_handle;
        // Certain entities at the start of our set are internal.
        // Remember the handle after we're done creating the internal entities,
        // so we can iterate through the non-internal ones, if we so desire.
        // This assumes all our internal entities will be contiguous and at the
        // start of our set.
        Handle first_non_internal_handle;
    };

    static constexpr Handle NO_ENTITY_HANDLE = 0;

    static Handle make_handle();
    static Entity * add_entity_to_set(char const *debug_name);
};
