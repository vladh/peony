#ifndef ASSET_H
#define ASSET_H

struct Asset {
public:
  const char *name;
  bool32 is_loaded = false;
};

#endif
