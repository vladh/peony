Task::Task(
  TaskType type,
  ModelAsset *model_asset,
  PersistentPbo *persistent_pbo,
  Memory *memory
) :
  type(type),
  model_asset(model_asset),
  persistent_pbo(persistent_pbo),
  memory(memory)
{
}

void Task::run() {
  START_TIMER(task_run);
  if (this->type == TASKTYPE_LOAD_MODEL) {
#if 0
    log_info(
      "Running `.load()` for model %s",
      this->model_asset->name
    );
#endif
    this->model_asset->load(this->memory);
  } else if (this->type ==  TASKTYPE_COPY_TEXTURES_TO_PBO) {
#if 0
    log_info(
      "Running `.copy_textures_to_pbo()` for model %s",
      this->model_asset->name
    );
#endif
    this->model_asset->copy_textures_to_pbo(this->persistent_pbo);
  } else {
    log_error("Don't know how to run task with type %d", this->type);
  }
  END_TIMER(task_run);
}
