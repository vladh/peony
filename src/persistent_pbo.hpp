class PersistentPbo {
public:
  uint32 pbo = 0;
  void *memory = nullptr;
  int32 width = 0;
  int32 height = 0;
  int32 n_components = 0;
  uint16 texture_count = 0;
  uint32 texture_size = 0;
  uint32 total_size = 0;

  PersistentPbo(
    uint16 texture_count, int32 width, int32 height, int32 n_components
  );
  uint16 get_new_idx();
  void* get_offset_for_idx(uint16 idx);
  void* get_memory_for_idx(uint16 idx);
  void allocate_pbo();

private:
  uint16 next_idx;
};
