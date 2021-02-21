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
  Models::ModelAsset *model_asset,
  Materials::PersistentPbo *persistent_pbo,
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
    Models::load_model_asset(task->model_asset, task->memory);
  } else if (task->type ==  TaskType::copy_textures_to_pbo) {
    Models::copy_textures_to_pbo(task->model_asset, task->persistent_pbo);
  } else {
    log_error("Don't know how to run task with type %d", task->type);
  }
  END_TIMER(run_task);
}


void Tasks::run_loading_loop(
  std::mutex *mutex, Memory *memory, State *state, uint32 idx_thread
) {
  while (!state->should_stop) {
    Tasks::Task *task = nullptr;

    mutex->lock();
    if (state->task_queue.size > 0) {
      task = state->task_queue.pop();
    }
    mutex->unlock();

    if (task) {
      log_info(
        "[Thread #%d] Running task %s for model %s",
        idx_thread,
        Tasks::task_type_to_str(task->type),
        task->model_asset->name
      );
      Tasks::run_task(task);
      log_info(
        "[Thread #%d] Finished task %s for model %s",
        idx_thread,
        Tasks::task_type_to_str(task->type),
        task->model_asset->name
      );
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
