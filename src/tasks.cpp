const char* Tasks::task_type_to_str(TaskType type) {
  if (type == TaskType::load_model) {
    return "load_model";
  } else if (type == TaskType::copy_textures_to_pbo) {
    return "copy_textures_to_pbo";
  }
  return "unknown";
}


void Tasks::run_task(Task *task) {
  START_TIMER(run_task);
  if (task->type == TaskType::load_model) {
    Models::load_model(task->target.entity_loader);
  } else if (task->type ==  TaskType::copy_textures_to_pbo) {
    Materials::copy_textures_to_pbo(task->target.material, task->persistent_pbo);
  } else {
    log_error("Don't know how to run task with type %d", task->type);
  }
  END_TIMER(run_task);
}


void Tasks::run_loading_loop(
  std::mutex *mutex, State *state, uint32 idx_thread
) {
  while (!state->should_stop) {
    Task *task = nullptr;

    mutex->lock();
    if (state->task_queue.size > 0) {
      task = state->task_queue.pop();
    }
    mutex->unlock();

    if (task) {
      if (task->type == TaskType::load_model) {
        log_info(
          "[Thread #%d] Running task %s for model %s",
          idx_thread,
          Tasks::task_type_to_str(task->type),
          task->target.entity_loader->name
        );
      }
      if (task->type == TaskType::copy_textures_to_pbo) {
        log_info(
          "[Thread #%d] Running task %s for material %s",
          idx_thread,
          Tasks::task_type_to_str(task->type),
          task->target.material->name
        );
      }

      Tasks::run_task(task);

      if (task->type == TaskType::load_model) {
        log_info(
          "[Thread #%d] Finished task %s for model %s",
          idx_thread,
          Tasks::task_type_to_str(task->type),
          task->target.entity_loader->name
        );
      }
      if (task->type == TaskType::copy_textures_to_pbo) {
        log_info(
          "[Thread #%d] Finished task %s for material %s",
          idx_thread,
          Tasks::task_type_to_str(task->type),
          task->target.material->name
        );
      }
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
