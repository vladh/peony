#include "pack.hpp"


void pack::init(Pack *pack) {
  *pack = 0ULL;
}


void pack::set(Pack *pack, uint8 value, uint8 idx) {
  assert(idx < MAX_LENGTH);
  Pack mask = 0b1111ULL << (idx * ELEMENT_SIZE);
  *pack = (*pack & ~mask) | (((Pack)value << (idx * ELEMENT_SIZE)) & mask);
}


uint8 pack::get(Pack *pack, uint8 idx) {
  Pack mask = 0b1111ULL << (idx * ELEMENT_SIZE);
  return (uint8)((*pack & mask) >> (idx * ELEMENT_SIZE));
}


uint8 pack::get_count(Pack *pack) {
  return get(pack, COUNTER_IDX);
}


void pack::push(Pack *pack, uint8 value) {
  uint8 count = get_count(pack);
  // Last element is reserved for count;
  assert(count < COUNTER_IDX);
  set(pack, value, count);
  set(pack, count + 1, COUNTER_IDX);
}
