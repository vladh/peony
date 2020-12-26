const char* Tasks::task_type_to_str(TaskType type) {
  if (type == TaskType::load_model) {
    return "load_model";
  } else if (type == TaskType::copy_textures_to_pbo) {
    return "copy_textures_to_pbo";
  }
  return "unknown";
}


Tasks::Task* Tasks::init_task(
  Task *task,
  TaskType type,
  ModelAsset *model_asset,
  Textures::PersistentPbo *persistent_pbo,
  Memory *memory
) {
  task->type = type;
  task->model_asset = model_asset;
  task->persistent_pbo = persistent_pbo;
  task->memory = memory;
  return task;
}


void Tasks::run_task(Tasks::Task *task) {
  START_TIMER(run_task);
  if (task->type == TaskType::load_model) {
    task->model_asset->load(task->memory);
  } else if (task->type ==  TaskType::copy_textures_to_pbo) {
    task->model_asset->copy_textures_to_pbo(task->persistent_pbo);
  } else {
    log_error("Don't know how to run task with type %d", task->type);
  }
  END_TIMER(run_task);
}
