#ifndef ARRAY_H
#define ARRAY_H

template <typename T>
class Array {
public:
  uint32 size = 0;
  uint32 max_size = 0;
  bool32 is_sparse = false;

  Array(
    MemoryPool *pool, uint32 max_size, const char *debug_name
  ) :
    max_size(max_size)
  {
    this->items = (T*)pool->push(sizeof(T) * this->max_size, debug_name);
  }

  Array(
    MemoryPool *pool, uint32 max_size, const char *debug_name,
    bool32 is_sparse
  ) :
    max_size(max_size),
    is_sparse(is_sparse)
  {
    this->items = (T*)pool->push(sizeof(T) * this->max_size, debug_name);
  }

  Array(
    MemoryPool *pool,
    uint32 size,
    uint32 max_size,
    T *items
  ) :
    size(size),
    max_size(max_size),
    items(items)
  {
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
    if (idx < 0) {
      return nullptr;
    }
    if (idx >= this->size) {
      if (this->is_sparse) {
        this->size = idx + 1;
      } else {
        return nullptr;
      }
    }
    return &this->items[idx];
  }

  T* operator[](uint32 idx) {
    return get(idx);
  }

  T* begin() {
    return this->items;
  }

  T* end() {
    return &this->items[this->size];
  }

  void set(uint32 idx, T *new_item) {
    *(this->items + idx) = *new_item;
  }

  T* get_items_ptr() {
    return this->items;
  }

private:
  T *items = nullptr;
};

#endif
