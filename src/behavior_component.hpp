#ifndef BEHAVIOR_COMPONENT_H
#define BEHAVIOR_COMPONENT_H

enum class Behavior {
  test
};

struct BehaviorComponent : public Component {
  Behavior behavior;
};

#endif
