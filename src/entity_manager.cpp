EntityManager::EntityManager(Array<Entities::Entity> *entities) {
  this->entities = entities;
}


Entities::Entity* EntityManager::add(const char* debug_name) {
  Entities::EntityHandle new_handle = make_handle();
  Entities::Entity *new_entity = entities->get(new_handle);
  new_entity->handle = new_handle;
  new_entity->debug_name = debug_name;
  return new_entity;
}


Entities::Entity* EntityManager::get(Entities::EntityHandle handle) {
  return entities->get(handle);
}


Entities::EntityHandle EntityManager::make_handle() {
  return last_handle++;
}
