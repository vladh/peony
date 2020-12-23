const char* behavior_to_string(Behavior behavior) {
  if (behavior == Behavior::none) {
    return "none";
  } else if (behavior == Behavior::test) {
    return "test";
  } else {
    log_error("Don't know how to convert Behavior to string: %d", behavior);
    return "<unknown>";
  }
}


Behavior behavior_from_string(const char *str) {
  if (strcmp(str, "none") == 0) {
    return Behavior::none;
  } else if (strcmp(str, "test") == 0) {
    return Behavior::test;
  } else {
    log_fatal("Could not parse Behavior: %s", str);
    return Behavior::none;
  }
}


bool32 BehaviorComponent::is_valid() {
  return this->behavior != Behavior::none;
}


BehaviorComponent::BehaviorComponent() {};


BehaviorComponent::BehaviorComponent(
  EntityHandle entity_handle,
  Behavior behavior
) :
  entity_handle(entity_handle),
  behavior(behavior)
{
}
