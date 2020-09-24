#define PACK_ELEMENT_SIZE 4
#define PACK_MAX_LENGTH (64 / PACK_ELEMENT_SIZE)
#define PACK_COUNTER_IDX (PACK_MAX_LENGTH - 1)

typedef uint64 Pack;

void pack_init(Pack *pack);
void pack_set(Pack *pack, uint8 value, uint8 idx);
uint8 pack_get(Pack *pack, uint8 idx);
uint8 pack_get_count(Pack *pack);
void pack_push(Pack *pack, uint8 value);
