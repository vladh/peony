#ifndef ARRAY_H
#define ARRAY_H

template <typename T>
class Array {
public:
  Array(MemoryPool *pool, uint32 new_max_size) {
    size = 0;
    max_size = new_max_size;
    items = (T*)memory_push_memory_to_pool(pool, sizeof(T) * max_size);
  }

  Array(MemoryPool *pool, uint32 new_size, uint32 new_max_size, T *new_items) {
    size = new_size;
    max_size = new_max_size;
    items = new_items;
  }

  T* push() {
    assert(size < max_size);
    T* new_item = items + size++;
    return new_item;
  }

  T push(T new_item) {
    assert(size < max_size);
    items[size++] = new_item;
    return new_item;
  }

  T* get(uint32 idx) {
    return &items[idx];
  }

  T* get_items_ptr() {
    return items;
  }

  uint32 get_size() {
    return size;
  }

  uint32 get_max_size() {
    return max_size;
  }

private:
  T *items;
  uint32 size;
  uint32 max_size;
};

#endif
