/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#pragma once

#include <GLFW/glfw3.h>
#include "types.hpp"

namespace core {
  struct WindowSize {
    int32 width; // in pixels (size of framebuffer)
    int32 height; // in pixels (size of framebuffer)
    uint32 screencoord_width; // in screen coordinates
    uint32 screencoord_height; // in screen coordinates
  };

  int run();
}

using core::WindowSize;
