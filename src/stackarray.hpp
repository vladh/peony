#ifndef STACKARRAY_HPP
#define STACKARRAY_HPP

template <typename T, uint32 new_capacity>
class StackArray {
public:
  T items[new_capacity] = {0};
  uint32 length = 0;
  uint32 capacity = new_capacity;
  bool32 is_sparse = false;
  uint32 starting_idx = 0;

  T* push() {
    assert(this->length < this->capacity);
    uint32 new_idx = this->length;
    this->length++;
    T* new_slot = &this->items[new_idx];
    return new_slot;
  }

  T* push(T new_item) {
    T* new_slot = push();
    *new_slot = new_item;
    return new_slot;
  }

  T* get(uint32 idx) {
    assert(idx >= this->starting_idx && idx < this->capacity);
    if (idx >= this->length) {
      assert(this->is_sparse);
      this->length = idx + 1;
    }
    return &this->items[idx];
  }

  T* operator[](uint32 idx) {
    return get(idx);
  }

  T* begin() {
    return &this->items[this->starting_idx];
  }

  T* end() {
    return &this->items[this->length];
  }

  void delete_elements_after_index(uint32 idx) {
    this->length = idx;
  }
};

#endif
