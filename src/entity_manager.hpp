#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

class EntityManager {
public:
  EntityManager(Array<Entity> *entities);

  Entity* add(const char* debug_name);

  Entity* get(EntityHandle handle);

private:
  EntityHandle last_handle = 0;

  Array<Entity> *entities;

  EntityHandle make_handle();
};

#endif
