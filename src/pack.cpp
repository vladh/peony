void pack_init(Pack *pack) {
  *pack = 0ULL;
}

void pack_set(Pack *pack, uint8 value, uint8 idx) {
  assert(idx < PACK_MAX_LENGTH);
  Pack mask = 0b1111ULL << (idx * PACK_ELEMENT_SIZE);
  *pack = (*pack & ~mask) | (((Pack)value << (idx * PACK_ELEMENT_SIZE)) & mask);
}

uint8 pack_get(Pack *pack, uint8 idx) {
  Pack mask = 0b1111ULL << (idx * PACK_ELEMENT_SIZE);
  return (uint8)((*pack & mask) >> (idx * PACK_ELEMENT_SIZE));
}

uint8 pack_get_count(Pack *pack) {
  return pack_get(pack, PACK_COUNTER_IDX);
}

void pack_push(Pack *pack, uint8 value) {
  uint8 count = pack_get_count(pack);
  // Last element is reserved for count;
  assert(count < PACK_COUNTER_IDX);
  pack_set(pack, value, count);
  pack_set(pack, count + 1, PACK_COUNTER_IDX);
}
