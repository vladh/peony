namespace entities {
  // -----------------------------------------------------------
  // Types
  // -----------------------------------------------------------
  // NOTE: 0 is an invalid handle.
  typedef uint32 EntityHandle;

  struct Entity {
    EntityHandle handle;
    char debug_name[MAX_DEBUG_NAME_LENGTH];
    static EntityHandle no_entity_handle;
  };

  EntityHandle Entity::no_entity_handle = 0;

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


  // -----------------------------------------------------------
  // Private functions
  // -----------------------------------------------------------
  EntityHandle make_handle(
    EntitySet *entity_set
  ) {
    // NOTE: 0 is an invalid handle.
    if (entity_set->next_handle == 0) {
      entity_set->next_handle++;
    }
    return entity_set->next_handle++;
  }


  Entity* add_entity_to_set(
    EntitySet *entity_set,
    const char *debug_name
  ) {
    EntityHandle new_handle = make_handle(entity_set);
    Entity *new_entity = entity_set->entities[new_handle];
    new_entity->handle = new_handle;
    strcpy(new_entity->debug_name, debug_name);
    return new_entity;
  }


  // -----------------------------------------------------------
  // Public functions
  // -----------------------------------------------------------
}

using entities::EntityHandle, entities::Entity, entities::EntitySet;
