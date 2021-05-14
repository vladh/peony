namespace tasks {
  // -----------------------------------------------------------
  // Types
  // -----------------------------------------------------------
  typedef void (*TaskFn)(void*, void*);
  struct Task {
    TaskFn fn;
    void *argument_1;
    void *argument_2;
  };


  // -----------------------------------------------------------
  // Private functions
  // -----------------------------------------------------------
  void run_task(Task *task) {
    auto t0 = debug_start_timer();
    task->fn(task->argument_1, task->argument_2);
    real64 duration = debug_end_timer(t0);
    logs::info("Task took %.0fms", duration);
  }


  void run_loading_loop(
    std::mutex *mutex,
    bool32 *should_stop,
    Queue<Task> *task_queue,
    uint32 idx_thread
  ) {
    while (!*should_stop) {
      Task *task = nullptr;

      mutex->lock();
      if (task_queue->size > 0) {
        task = task_queue->pop();
      }
      mutex->unlock();

      if (task) {
        run_task(task);
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }


  // -----------------------------------------------------------
  // Public functions
  // -----------------------------------------------------------
}

using tasks::Task, tasks::TaskFn;
