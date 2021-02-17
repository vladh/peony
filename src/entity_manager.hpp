#ifndef ENTITY_MANAGER_HPP
#define ENTITY_MANAGER_HPP

class EntityManager {
public:
  // NOTE: 0 is an invalid handle.
  Entities::EntityHandle last_handle = 1;
  Array<Entities::Entity> *entities;

  Entities::EntityHandle make_handle();
  Entities::Entity* add(const char* debug_name);
  Entities::Entity* get(Entities::EntityHandle handle);
  EntityManager(Array<Entities::Entity> *entities);
};

#endif
