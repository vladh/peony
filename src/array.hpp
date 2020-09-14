#ifndef ARRAY_H
#define ARRAY_H

template <typename T>
struct Array {
  T *items;
  uint32 size;
  uint32 max_size;
};

template <typename T>
T* array_push(Array<T> *array) {
  assert(array->size < array->max_size);
  T* new_item = array->items + array->size++;
  return new_item;
}

template <typename T>
T array_push(Array<T> *array, T new_item) {
  assert(array->size < array->max_size);
  array->items[array->size++] = new_item;
  return new_item;
}

template <typename T>
void array_init(MemoryPool *pool, Array<T> *array, uint32 max_size) {
  array->size = 0;
  array->max_size = max_size;
  array->items = (T*)memory_push_memory_to_pool(
    pool, sizeof(T) * array->max_size
  );
}

#endif

