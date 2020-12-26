#ifndef TASK_HPP
#define TASK_HPP

class ModelAsset;
class Memory;

namespace Tasks {
  enum class TaskType {load_model, copy_textures_to_pbo};

  struct Task {
    TaskType type;
    ModelAsset *model_asset;
    Textures::PersistentPbo *persistent_pbo;
    Memory *memory;
  };

  static const char* task_type_to_str(TaskType type);
  void run_task(Task *task);
  Task* init_task(
    Task *task,
    TaskType type,
    ModelAsset *model_asset,
    Textures::PersistentPbo *persistent_pbo,
    Memory *memory
  );
};

#endif
