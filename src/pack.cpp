// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#include "pack.hpp"


void
pack::init(Pack *pack)
{
    *pack = 0ULL;
}


void
pack::set(Pack *pack, u8 value, u8 idx)
{
    assert(idx < MAX_LENGTH);
    Pack mask = 0b1111ULL << (idx * ELEMENT_SIZE);
    *pack = (*pack & ~mask) | (((Pack)value << (idx * ELEMENT_SIZE)) & mask);
}


u8
pack::get(Pack *pack, u8 idx)
{
    Pack mask = 0b1111ULL << (idx * ELEMENT_SIZE);
    return (u8)((*pack & mask) >> (idx * ELEMENT_SIZE));
}


u8
pack::get_count(Pack *pack)
{
    return get(pack, COUNTER_IDX);
}


void
pack::push(Pack *pack, u8 value)
{
    u8 count = get_count(pack);
    // Last element is reserved for count;
    assert(count < COUNTER_IDX);
    set(pack, value, count);
    set(pack, count + 1, COUNTER_IDX);
}
