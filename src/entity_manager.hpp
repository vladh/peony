#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

class EntityManager {
public:
  EntityManager(Array<Entity> *entities);

  Entity* add(const char* debug_name);

  Entity* get(EntityHandle handle);

private:
  EntityHandle last_handle = 0;
  // TODO: std::unordered_map is #slow, find a better implementation.
  // https://stackoverflow.com/questions/3300525/super-high-performance-c-c-hash-map-table-dictionary
  std::unordered_map<EntityHandle, Entity*> entity_handle_map;

  Array<Entity> *entities;

  EntityHandle make_handle();
};

#endif
