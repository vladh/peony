#ifndef ENTITY_MANAGER_HPP
#define ENTITY_MANAGER_HPP

class EntityManager {
public:
  EntityManager(Array<Entity> *entities);

  Entity* add(const char* debug_name);

  Entity* get(EntityHandle handle);

private:
  // NOTE: 0 is an invalid handle.
  EntityHandle last_handle = 1;

  Array<Entity> *entities;

  EntityHandle make_handle();
};

#endif
