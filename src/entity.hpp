#ifndef ENTITY_H
#define ENTITY_H

typedef uint32 EntityHandle;

class Entity {
public:
  EntityHandle handle;
  const char *debug_name;

  static EntityHandle no_entity_handle;
};

EntityHandle Entity::no_entity_handle = 0;

#endif
