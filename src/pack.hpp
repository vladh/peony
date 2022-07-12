// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"

class pack {
public:
    static constexpr uint16 ELEMENT_SIZE = 4;
    static constexpr uint16 MAX_LENGTH  = (64 / ELEMENT_SIZE);
    static constexpr uint16 COUNTER_IDX  = (MAX_LENGTH - 1);

    typedef uint64 Pack;

    static void init(Pack *pack);
    static void set(Pack *pack, uint8 value, uint8 idx);
    static uint8 get(Pack *pack, uint8 idx);
    static uint8 get_count(Pack *pack);
    static void push(Pack *pack, uint8 value);
};
