void pack::init(Pack *pack) {
  *pack = 0ULL;
}


void pack::set(Pack *pack, uint8 value, uint8 idx) {
  assert(idx < PACK_MAX_LENGTH);
  Pack mask = 0b1111ULL << (idx * PACK_ELEMENT_SIZE);
  *pack = (*pack & ~mask) | (((Pack)value << (idx * PACK_ELEMENT_SIZE)) & mask);
}


uint8 pack::get(Pack *pack, uint8 idx) {
  Pack mask = 0b1111ULL << (idx * PACK_ELEMENT_SIZE);
  return (uint8)((*pack & mask) >> (idx * PACK_ELEMENT_SIZE));
}


uint8 pack::get_count(Pack *pack) {
  return pack::get(pack, PACK_COUNTER_IDX);
}


void pack::push(Pack *pack, uint8 value) {
  uint8 count = pack::get_count(pack);
  // Last element is reserved for count;
  assert(count < PACK_COUNTER_IDX);
  pack::set(pack, value, count);
  pack::set(pack, count + 1, PACK_COUNTER_IDX);
}
