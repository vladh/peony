#pragma once

#include <mutex>
#include "types.hpp"

namespace tasks {
  typedef void (*TaskFn)(void*, void*);
  struct Task {
    TaskFn fn;
    void *argument_1;
    void *argument_2;
  };

  void run_loading_loop(
    std::mutex *mutex,
    bool32 *should_stop,
    Queue<Task> *task_queue,
    uint32 idx_thread
  );
}

using tasks::Task, tasks::TaskFn;
