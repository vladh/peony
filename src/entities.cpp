// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "entities.hpp"
#include "engine.hpp"


EntityHandle entities::make_handle() {
  EntitySet *entity_set = engine::get_entity_set();
  // NOTE: 0 is an invalid handle.
  if (entity_set->next_handle == 0) {
    entity_set->next_handle++;
  }
  return entity_set->next_handle++;
}


Entity* entities::add_entity_to_set(const char *debug_name) {
  EntitySet *entity_set = engine::get_entity_set();
  EntityHandle new_handle = make_handle();
  Entity *new_entity = entity_set->entities[new_handle];
  new_entity->handle = new_handle;
  strcpy(new_entity->debug_name, debug_name);
  return new_entity;
}
