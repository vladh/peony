#ifndef BEHAVIOR_COMPONENT_MANAGER_HPP
#define BEHAVIOR_COMPONENT_MANAGER_HPP

class BehaviorComponentManager {
public:
  Array<Entities::BehaviorComponent> *components;

  Entities::BehaviorComponent* add(
    Entities::BehaviorComponent behavior_component
  );
  Entities::BehaviorComponent* add(
    Entities::EntityHandle entity_handle,
    Entities::Behavior behavior
  );
  Entities::BehaviorComponent* get(Entities::EntityHandle handle);
  BehaviorComponentManager(
    Array<Entities::BehaviorComponent> *components
  );
};

#endif
