#ifndef ENTITY_HPP
#define ENTITY_HPP

typedef uint32 EntityHandle;

struct Entity {
  EntityHandle handle;
  const char *debug_name;
  static EntityHandle no_entity_handle;
};

EntityHandle Entity::no_entity_handle = 0;

#endif
