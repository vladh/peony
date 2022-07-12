// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include <mutex>
#include "types.hpp"
#include "queue.hpp"

class tasks {
public:
    typedef void (*TaskFn)(void*);
    struct Task {
        TaskFn fn;
        void *argument_1;
    };
    struct State {
        Queue<Task> task_queue;
    };

    static void push(Task task);
    static void run_loading_loop(
        std::mutex *mutex,
        bool32 *should_stop,
        uint32 idx_thread
    );
    static void init(tasks::State *tasks_state, memory::Pool *pool);

private:
    static void run_task(Task *task);

    static tasks::State *state;
};
