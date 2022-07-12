// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

struct WindowSize {
    i32 width; // in pixels (size of framebuffer)
    i32 height; // in pixels (size of framebuffer)
    u32 screencoord_width; // in screen coordinates
    u32 screencoord_height; // in screen coordinates
};

