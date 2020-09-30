EntityManager::EntityManager(Array<Entity> *entities) {
  this->entities = entities;
}


Entity* EntityManager::add(const char* debug_name) {
  Entity *new_entity = array_push<Entity>(entities);
  new_entity->handle = make_handle();
  new_entity->debug_name = debug_name;
  return new_entity;
}


Entity* EntityManager::get(EntityHandle handle) {
  // NOTE: Normally we'd use a hash-map or something here, but
  // std::unordered_map is slow as heck. This nice ol' array is faster.
  // Let's look for something else if this starts showing up in the profiler.
  for (uint32 idx = 0; idx < entities->size; idx++) {
    if (entities->items[idx].handle == handle) {
      return &entities->items[idx];
    }
  }
  return nullptr;
}


EntityHandle EntityManager::make_handle() {
  return last_handle++;
}
