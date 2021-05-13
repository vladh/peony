#ifndef QUEUE_HPP
#define QUEUE_HPP

template <typename T>
class Queue {
public:
  uint32 size = 0;
  uint32 max_size = 0;
  T *items = nullptr;
  uint32 head = 0;
  uint32 tail = 0;

  T* push() {
    assert(this->size < this->max_size);
    T* new_slot = this->items + this->tail;
    if (this->tail + 1 >= this->max_size) {
      this->tail = 0;
    } else {
      this->tail++;
    }
    this->size++;
    return new_slot;
  }

  T push(T new_item) {
    T* new_slot = push();
    *new_slot = new_item;
    return new_item;
  }

  T* pop() {
    assert(this->size > 0);
    T* item = this->items + this->head;
    if (this->head + 1 >= this->max_size) {
      this->head = 0;
    } else {
      this->head++;
    }
    this->size--;
    return item;
  }

  Queue(MemoryPool *memory_pool, uint32 new_max_size, const char *debug_name) {
    this->max_size = new_max_size;
    this->items = (T*)memory::push(memory_pool, sizeof(T) * this->max_size, debug_name);
  }

  Queue(MemoryPool *memory_pool, uint32 new_size, uint32 new_max_size, T *new_items) {
    this->size = new_size;
    this->head = 0;
    this->tail = new_size;
    this->max_size = new_max_size;
    this->items = new_items;
  }
};

#endif
