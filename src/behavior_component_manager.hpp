#ifndef BEHAVIOR_COMPONENT_MANAGER_H
#define BEHAVIOR_COMPONENT_MANAGER_H

class BehaviorComponentManager {
public:
  Array<BehaviorComponent> *components;

  BehaviorComponentManager(
    Array<BehaviorComponent> *components
  );

  BehaviorComponent* add(
    EntityHandle entity_handle,
    Behavior behavior
  );

  BehaviorComponent* get(EntityHandle handle);
};

#endif
