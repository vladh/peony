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
  if (this->type == TaskType::load_model) {
#if 1
    log_info(
      "(task start) `.load()` for model %s",
      this->model_asset->name
    );
#endif
    this->model_asset->load(this->memory);
#if 1
    log_info(
      "(task end) running `.load()` for model %s",
      this->model_asset->name
    );
#endif
  } else if (this->type ==  TaskType::copy_textures_to_pbo) {
#if 1
    log_info(
      "(task start) `.copy_textures_to_pbo()` for model %s",
      this->model_asset->name
    );
#endif
    this->model_asset->copy_textures_to_pbo(this->persistent_pbo);
#if 1
    log_info(
      "(task end) running `.copy_textures_to_pbo()` for model %s",
      this->model_asset->name
    );
#endif
  } else {
    log_error("Don't know how to run task with type %d", this->type);
  }
  END_TIMER(task_run);
}
