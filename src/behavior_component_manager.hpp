#ifndef BEHAVIOR_COMPONENT_MANAGER_HPP
#define BEHAVIOR_COMPONENT_MANAGER_HPP

class BehaviorComponentManager {
public:
  Array<BehaviorComponent> *components;

  BehaviorComponent* add(
    BehaviorComponent behavior_component
  );

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
