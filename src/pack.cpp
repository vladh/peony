namespace pack {
  // -----------------------------------------------------------
  // Constants
  // -----------------------------------------------------------
  constexpr uint16 ELEMENT_SIZE = 4;
  constexpr uint16 MAX_LENGTH  = (64 / ELEMENT_SIZE);
  constexpr uint16 COUNTER_IDX  = (MAX_LENGTH - 1);


  // -----------------------------------------------------------
  // Types
  // -----------------------------------------------------------
  typedef uint64 Pack;


  // -----------------------------------------------------------
  // Public functions
  // -----------------------------------------------------------
  void init(Pack *pack) {
    *pack = 0ULL;
  }


  void set(Pack *pack, uint8 value, uint8 idx) {
    assert(idx < MAX_LENGTH);
    Pack mask = 0b1111ULL << (idx * ELEMENT_SIZE);
    *pack = (*pack & ~mask) | (((Pack)value << (idx * ELEMENT_SIZE)) & mask);
  }


  uint8 get(Pack *pack, uint8 idx) {
    Pack mask = 0b1111ULL << (idx * ELEMENT_SIZE);
    return (uint8)((*pack & mask) >> (idx * ELEMENT_SIZE));
  }


  uint8 get_count(Pack *pack) {
    return get(pack, COUNTER_IDX);
  }


  void push(Pack *pack, uint8 value) {
    uint8 count = get_count(pack);
    // Last element is reserved for count;
    assert(count < COUNTER_IDX);
    set(pack, value, count);
    set(pack, count + 1, COUNTER_IDX);
  }
}

using pack::Pack;
