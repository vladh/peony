#ifndef ARRAY_HPP
#define ARRAY_HPP

template <typename T>
class Array {
public:
  MemoryPool *memory_pool = nullptr;
  const char *debug_name = nullptr;
  uint32 size = 0;
  uint32 max_size = 0;
  bool32 is_sparse = false;
  T *items = nullptr;

  void alloc() {
    this->items = (T*)Memory::push(
      this->memory_pool,
      sizeof(T) * this->max_size,
      this->debug_name
    );
  }

  T* push() {
    if (!this->items) {
      alloc();
    }
    assert(this->size < this->max_size);
    uint32 new_idx = this->size;
    T* new_slot = &this->items[new_idx];
    this->size++;
    return new_slot;
  }

  T push(T new_item) {
    T* new_slot = push();
    *new_slot = new_item;
    return new_item;
  }

  T* get(uint32 idx) {
    assert(idx >= 0 && idx < this->max_size);
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

  void delete_elements_after_index(uint32 idx) {
    this->size = idx;
  }

  Array(
    MemoryPool *memory_pool,
    uint32 max_size,
    const char *debug_name,
    bool32 is_sparse = false
  ) :
    memory_pool(memory_pool),
    debug_name(debug_name),
    max_size(max_size),
    is_sparse(is_sparse)
  {
    alloc();
  }
};

#endif
