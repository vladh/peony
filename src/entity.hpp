#ifndef ENTITY_H
#define ENTITY_H

typedef uint32 EntityHandle;

class Entity {
public:
  EntityHandle handle;
  const char *debug_name;
  void print();
};

#endif
