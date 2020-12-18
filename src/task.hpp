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
  static const char* task_type_to_str(TaskType type) {
    if (type == TaskType::load_model) {
      return "load_model";
    } else if (type == TaskType::copy_textures_to_pbo) {
      return "copy_textures_to_pbo";
    }
    return "unknown";
  }
  Task(
    TaskType type,
    ModelAsset *model_asset,
    PersistentPbo *persistent_pbo,
    Memory *memory
  );
};

#endif
