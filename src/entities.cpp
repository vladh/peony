// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "entities.hpp"
#include "engine.hpp"


entities::State *entities::state = nullptr;


entities::Handle
entities::make_handle()
{
    if (entities::state->next_handle == NO_ENTITY_HANDLE) {
        entities::state->next_handle++;
    }
    return entities::state->next_handle++;
}


entities::Entity *
entities::add_entity_to_set(char const *debug_name)
{
    Handle new_handle = make_handle();
    Entity *new_entity = get_entity(new_handle);
    new_entity->handle = new_handle;
    strcpy(new_entity->debug_name, debug_name);
    return new_entity;
}


void
entities::mark_first_non_internal_handle()
{
    entities::state->first_non_internal_handle = entities::state->next_handle;
}


entities::Handle
entities::get_first_non_internal_handle()
{
    return entities::state->first_non_internal_handle;
}


void
entities::destroy_non_internal_entities()
{
    for (
        u32 idx = entities::state->first_non_internal_handle;
        idx < entities::state->entities.length;
        idx++
    ) {
        drawable::destroy_component(drawable::get_component(idx));
    }

    entities::state->next_handle = entities::state->first_non_internal_handle;

    entities::state->entities.delete_elements_after_index(
        entities::state->first_non_internal_handle);

    lights::get_components()->delete_elements_after_index(
        entities::state->first_non_internal_handle);
    spatial::get_components()->delete_elements_after_index(
        entities::state->first_non_internal_handle);
    drawable::get_components()->delete_elements_after_index(
        entities::state->first_non_internal_handle);
    behavior::get_components()->delete_elements_after_index(
        entities::state->first_non_internal_handle);
    anim::get_components()->delete_elements_after_index(
        entities::state->first_non_internal_handle);
}


u32
entities::get_n_entities()
{
    return entities::state->entities.length;
}


Array<entities::Entity> *
entities::get_entities()
{
    return &entities::state->entities;
}


entities::Entity *
entities::get_entity(entities::Handle entity_handle)
{
    return entities::state->entities[entity_handle];
}


void
entities::init(entities::State *entities_state, memory::Pool *asset_memory_pool)
{
    entities::state = entities_state;
    entities::state->entities = Array<entities::Entity>(
        asset_memory_pool, MAX_N_ENTITIES, "entities", true, 1);
}
