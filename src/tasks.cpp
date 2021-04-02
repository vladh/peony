const char* Tasks::task_type_to_str(TaskType type) {
  if (type == TaskType::load_model) {
    return "load_model";
  } else if (type == TaskType::copy_textures_to_pbo) {
    return "copy_textures_to_pbo";
  }
  return "unknown";
}


void Tasks::run_task(Task *task) {
  auto t0 = debug_start_timer();

  if (task->type == TaskType::load_model) {
    Models::load_model(task->target.entity_loader);
  } else if (task->type ==  TaskType::copy_textures_to_pbo) {
    Materials::copy_textures_to_pbo(task->target.material, task->persistent_pbo);
  } else {
    log_error("Don't know how to run task with type %d", task->type);
  }

  real64 duration = debug_end_timer(t0);
  log_info("Task %s took %.0fms", task_type_to_str(task->type), duration);
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
#if 0
      if (task->type == TaskType::load_model) {
        log_info(
          "[Thread #%d] Running task %s for model %s",
          idx_thread,
          task_type_to_str(task->type),
          task->target.entity_loader->name
        );
      }
      if (task->type == TaskType::copy_textures_to_pbo) {
        log_info(
          "[Thread #%d] Running task %s for material %s",
          idx_thread,
          task_type_to_str(task->type),
          task->target.material->name
        );
      }
#endif

      run_task(task);

#if 0
      if (task->type == TaskType::load_model) {
        log_info(
          "[Thread #%d] Finished task %s for model %s",
          idx_thread,
          task_type_to_str(task->type),
          task->target.entity_loader->name
        );
      }
      if (task->type == TaskType::copy_textures_to_pbo) {
        log_info(
          "[Thread #%d] Finished task %s for material %s",
          idx_thread,
          task_type_to_str(task->type),
          task->target.material->name
        );
      }
#endif
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
