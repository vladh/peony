#ifndef STACKARRAY_HPP
#define STACKARRAY_HPP

template <typename T, uint32 capacity>
class StackArray {
public:
  T items[capacity] = {{}};
  uint32 length = 0;
  bool32 is_sparse = false;
  uint32 starting_idx = 0;

  T* push() {
    assert(this->length < capacity);
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
    assert(idx >= this->starting_idx && idx < capacity);
    if (idx >= this->length) {
      assert(this->is_sparse);
      this->length = idx + 1;
    }
    return &this->items[idx];
  }

  T* operator[](uint32 idx) {
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
    memset(this->items, 0, sizeof(this->items));
    this->length = 0;
  }

  void delete_elements_after_index(uint32 idx) {
    memset(&this->items[idx], 0, sizeof(T) * (this->length - idx));
    this->length = idx;
  }
};

#endif
