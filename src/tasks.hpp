#pragma once

#include <mutex>
#include "types.hpp"
#include "queue.hpp"

namespace tasks {
  typedef void (*TaskFn)(void*, void*);
  struct Task {
    TaskFn fn;
    void *argument_1;
    void *argument_2;
  };
  struct TasksState {
    Queue<Task> task_queue;
  };

  void run_loading_loop(
    TasksState *tasks_state,
    std::mutex *mutex,
    bool32 *should_stop,
    uint32 idx_thread
  );
  void init(TasksState *tasks_state, MemoryPool *pool);
}

using tasks::Task, tasks::TaskFn, tasks::TasksState;
