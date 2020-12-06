#ifndef BEHAVIOR_COMPONENT_MANAGER_H
#define BEHAVIOR_COMPONENT_MANAGER_H

class BehaviorComponentManager {
public:
  Array<BehaviorComponent> *components;

  BehaviorComponent* add(
    EntityHandle entity_handle,
    Behavior behavior
  );

  BehaviorComponent* get(EntityHandle handle);

  BehaviorComponentManager(
    Array<BehaviorComponent> *components
  );
};

#endif
