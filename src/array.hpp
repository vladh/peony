#ifndef ARRAY_H
#define ARRAY_H

template <typename T>
class Array {
public:
  Array(MemoryPool *pool, uint32 new_max_size, const char *debug_name) {
    this->max_size = new_max_size;
    this->items = (T*)pool->push(sizeof(T) * this->max_size, debug_name);
  }

  Array(MemoryPool *pool, uint32 new_size, uint32 new_max_size, T *new_items) {
    this->size = new_size;
    this->max_size = new_max_size;
    this->items = new_items;
  }

  T* push() {
    assert(this->size < this->max_size);
    uint32 new_idx = this->size;
    T* new_slot = this->items + new_idx;
    this->size++;
    return new_slot;
  }

  T push(T new_item) {
    T* new_slot = push();
    *new_slot = new_item;
    return new_item;
  }

  T* get(uint32 idx) {
    return &this->items[idx];
  }

  T* get_items_ptr() {
    return this->items;
  }

  uint32 get_size() {
    return this->size;
  }

  uint32 get_max_size() {
    return this->max_size;
  }

private:
  T *items = nullptr;
  uint32 size = 0;
  uint32 max_size = 0;
};

#endif
