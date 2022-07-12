// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "entities.hpp"
#include "engine.hpp"


entities::Handle
entities::make_handle()
{
    entities::Set *entity_set = engine::get_entity_set();
    if (entity_set->next_handle == NO_ENTITY_HANDLE) {
        entity_set->next_handle++;
    }
    return entity_set->next_handle++;
}


entities::Entity *
entities::add_entity_to_set(char const *debug_name)
{
    entities::Set *entity_set = engine::get_entity_set();
    Handle new_handle = make_handle();
    Entity *new_entity = entity_set->entities[new_handle];
    new_entity->handle = new_handle;
    strcpy(new_entity->debug_name, debug_name);
    return new_entity;
}
