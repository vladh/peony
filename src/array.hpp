#ifndef ARRAY_H
#define ARRAY_H

#include "types.hpp"


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

#endif

