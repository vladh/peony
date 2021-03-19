#ifndef TASK_HPP
#define TASK_HPP

struct EntityLoader;

enum class TaskType {
  load_model,
  copy_textures_to_pbo,
};

union TaskTarget {
  Material *material;
  EntityLoader *entity_loader;
};

struct Task {
  TaskType type;
  TaskTarget target;
  PersistentPbo *persistent_pbo;
};

namespace Tasks {
  static const char* task_type_to_str(TaskType type);
  void run_task(Task *task);
  void run_loading_loop(
    std::mutex *mutex, State *state, uint32 idx_thread
  );
};

#endif
