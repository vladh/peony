#pragma once

namespace pack {
  constexpr uint16 ELEMENT_SIZE = 4;
  constexpr uint16 MAX_LENGTH  = (64 / ELEMENT_SIZE);
  constexpr uint16 COUNTER_IDX  = (MAX_LENGTH - 1);

  typedef uint64 Pack;

  void init(Pack *pack);
  void set(Pack *pack, uint8 value, uint8 idx);
  uint8 get(Pack *pack, uint8 idx);
  uint8 get_count(Pack *pack);
  void push(Pack *pack, uint8 value);
}

using pack::Pack;
