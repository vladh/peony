#ifndef ASSET_H
#define ASSET_H

struct Asset {
  const char *name;
  bool32 is_loaded = false;
};

#endif
