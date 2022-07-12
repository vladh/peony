// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include <thread>
#include "debug.hpp"
#include "logs.hpp"
#include "queue.hpp"
#include "tasks.hpp"
#include "intrinsics.hpp"


tasks::State *tasks::state = nullptr;


void
tasks::push(Task task)
{
    tasks::state->task_queue.push(task);
}


void
tasks::run_loading_loop(
    std::mutex *mutex,
    bool32 *should_stop,
    uint32 idx_thread
) {
    while (!*should_stop) {
        Task *task = nullptr;

        mutex->lock();
        if (tasks::state->task_queue.size > 0) {
            task = tasks::state->task_queue.pop();
        }
        mutex->unlock();

        if (task) {
            run_task(task);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}


void
tasks::init(tasks::State *tasks_state, memory::Pool *pool)
{
    tasks::state = tasks_state;
    tasks::state->task_queue = Queue<Task>(pool, 128, "task_queue");
}


void
tasks::run_task(Task *task)
{
    auto t0 = debug_start_timer();
    task->fn(task->argument_1);
    real64 duration = debug_end_timer(t0);
    logs::info("Task took %.0fms", duration);
}
