#ifndef TASK_HPP
#define TASK_HPP

class Memory;
namespace Models {
  struct ModelAsset;
};

namespace Tasks {
  enum class TaskType {load_model, copy_textures_to_pbo};

  struct Task {
    TaskType type;
    Models::ModelAsset *model_asset;
    Materials::PersistentPbo *persistent_pbo;
    Memory *memory;
  };

  static const char* task_type_to_str(TaskType type);
  void run_task(Task *task);
  Task* init_task(
    Task *task,
    TaskType type,
    Models::ModelAsset *model_asset,
    Materials::PersistentPbo *persistent_pbo,
    Memory *memory
  );
  void run_loading_loop(
    std::mutex *mutex, Memory *memory, State *state, uint32 idx_thread
  );
};

#endif
