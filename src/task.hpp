#ifndef TASK_H
#define TASK_H

class ModelAsset;
class PersistentPbo;
class Memory;

enum TaskType {
  TASKTYPE_LOAD_MODEL,
  TASKTYPE_COPY_TEXTURES_TO_PBO
};

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
