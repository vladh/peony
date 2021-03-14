#ifndef TASK_HPP
#define TASK_HPP

namespace Models {
  struct ModelAsset;
};

namespace Tasks {
  enum class TaskType {load_model, copy_textures_to_pbo};

  struct Task {
    TaskType type;
    Models::ModelAsset *model_asset;
    Materials::PersistentPbo *persistent_pbo;
  };

  static const char* task_type_to_str(TaskType type);
  void run_task(Task *task);
  Task* init_task(
    Task *task,
    TaskType type,
    Models::ModelAsset *model_asset,
    Materials::PersistentPbo *persistent_pbo
  );
  void run_loading_loop(
    std::mutex *mutex, State *state, uint32 idx_thread
  );
};

#endif
