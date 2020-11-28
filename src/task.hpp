#ifndef TASK_H
#define TASK_H

class ModelAsset;
class PersistentPbo;
class Memory;

enum class TaskType {load_model, copy_textures_to_pbo};

class Task {
public:
  Task(
    TaskType type,
    ModelAsset *model_asset,
    PersistentPbo *persistent_pbo,
    Memory *memory
  );
  void run();

private:
  TaskType type;
  ModelAsset *model_asset;
  PersistentPbo *persistent_pbo;
  Memory *memory;
};

#endif
