EntityManager::EntityManager(Array<Entity> *entities) {
  this->entities = entities;
}


Entity* EntityManager::add(const char* debug_name) {
  EntityHandle new_handle = make_handle();
  Entity *new_entity = entities->get(new_handle);
  new_entity->handle = new_handle;
  new_entity->debug_name = debug_name;
  return new_entity;
}


Entity* EntityManager::get(EntityHandle handle) {
  return entities->get(handle);
}


EntityHandle EntityManager::make_handle() {
  return last_handle++;
}
