EntityManager::EntityManager(Array<Entity> *entities) {
  this->entities = entities;
}


Entity* EntityManager::add(const char* debug_name) {
  Entity *new_entity = array_push<Entity>(entities);
  new_entity->handle = make_handle();
  new_entity->debug_name = debug_name;
  entity_handle_map[new_entity->handle] = new_entity;
  return new_entity;
}


Entity* EntityManager::get(EntityHandle handle) {
  if (entity_handle_map.count(handle)) {
    return entity_handle_map[handle];
  } else {
    return nullptr;
  }
}


EntityHandle EntityManager::make_handle() {
  return last_handle++;
}
