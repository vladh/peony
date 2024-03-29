// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "memory.hpp"

template <typename T>
class Array {
public:
    memory::Pool *memory_pool = nullptr;
    const char *debug_name = nullptr;
    u32 length = 0;
    u32 capacity = 0;
    bool is_sparse = false;
    u32 starting_idx = 0;
    T *items = nullptr;

    void alloc() {
        this->items = (T*)memory::push(this->memory_pool, sizeof(T) * this->capacity, this->debug_name);
    }

    T* push() {
        if (!this->items) {
            alloc();
        }
        assert(this->length < this->capacity);
        u32 new_idx = this->length;
        this->length++;
        T* new_slot = &this->items[new_idx];
        return new_slot;
    }

    T* push(T new_item) {
        T* new_slot = push();
        *new_slot = new_item;
        return new_slot;
    }

    T* get(u32 idx) {
        if (!this->items) {
            alloc();
        }
        assert(idx >= this->starting_idx && idx < this->capacity);
        if (idx >= this->length) {
            assert(this->is_sparse);
            this->length = idx + 1;
        }
        return &this->items[idx];
    }

    T* operator[](u32 idx) {
        return get(idx);
    }

    template <typename F>
        T* find(F match) {
            for (auto item = begin(); item < end(); item++) {
                if (match(item)) {
                    return item;
                }
            }
            return nullptr;
        }

    T* begin() {
        return &this->items[this->starting_idx];
    }

    T* end() {
        return &this->items[this->length];
    }

    void clear() {
        memset(this->items, 0, sizeof(T) * this->capacity);
        this->length = 0;
    }

    void delete_elements_after_index(u32 idx) {
        memset(&this->items[idx], 0, sizeof(T) * (this->length - idx));
        this->length = idx;
    }

    Array(
        memory::Pool *memory_pool,
        u32 capacity,
        const char *debug_name,
        bool is_sparse = false,
        u32 starting_idx = 0
    ) :
        memory_pool(memory_pool),
        debug_name(debug_name),
        capacity(capacity),
        is_sparse(is_sparse),
        starting_idx(starting_idx)
    {
    }
};
