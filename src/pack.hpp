// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"

class pack {
public:
    static constexpr u16 ELEMENT_SIZE = 4;
    static constexpr u16 MAX_LENGTH  = (64 / ELEMENT_SIZE);
    static constexpr u16 COUNTER_IDX  = (MAX_LENGTH - 1);

    typedef u64 Pack;

    static void init(Pack *pack);
    static void set(Pack *pack, u8 value, u8 idx);
    static u8 get(Pack *pack, u8 idx);
    static u8 get_count(Pack *pack);
    static void push(Pack *pack, u8 value);
};
