#ifndef TASK_HPP
#define TASK_HPP

class ModelAsset;
class PersistentPbo;
class Memory;

enum class TaskType {load_model, copy_textures_to_pbo};

class Task {
public:
  TaskType type;
  ModelAsset *model_asset;
  PersistentPbo *persistent_pbo;
  Memory *memory;

  void run();
  static const char* task_type_to_str(TaskType type);
  Task(
    TaskType type,
    ModelAsset *model_asset,
    PersistentPbo *persistent_pbo,
    Memory *memory
  );
};

#endif
