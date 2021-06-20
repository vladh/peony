/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#include <thread>
#include "debug.hpp"
#include "logs.hpp"
#include "queue.hpp"
#include "tasks.hpp"
#include "intrinsics.hpp"


namespace tasks {
  pny_internal void run_task(Task *task) {
    auto t0 = debug_start_timer();
    task->fn(task->argument_1, task->argument_2);
    real64 duration = debug_end_timer(t0);
    logs::info("Task took %.0fms", duration);
  }
}


void tasks::run_loading_loop(
  TasksState *tasks_state,
  std::mutex *mutex,
  bool32 *should_stop,
  uint32 idx_thread
) {
  while (!*should_stop) {
    Task *task = nullptr;

    mutex->lock();
    if (tasks_state->task_queue.size > 0) {
      task = tasks_state->task_queue.pop();
    }
    mutex->unlock();

    if (task) {
      run_task(task);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}


void tasks::init(TasksState *tasks_state, MemoryPool *pool) {
  tasks_state->task_queue = Queue<Task>(pool, 128, "task_queue");
}
