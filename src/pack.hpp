#ifndef PACK_HPP
#define PACK_HPP

typedef uint64 Pack;

namespace pack {
  constexpr uint16 PACK_ELEMENT_SIZE = 4;
  constexpr uint16 PACK_MAX_LENGTH  = (64 / PACK_ELEMENT_SIZE);
  constexpr uint16 PACK_COUNTER_IDX  = (PACK_MAX_LENGTH - 1);

  void init(Pack *pack);
  void set(Pack *pack, uint8 value, uint8 idx);
  uint8 get(Pack *pack, uint8 idx);
  uint8 get_count(Pack *pack);
  void push(Pack *pack, uint8 value);
}

#endif
