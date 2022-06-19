/*
  Peony Game Engine
  Copyright (C) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>
  All rights reserved.
*/

#pragma once

#include <GLFW/glfw3.h>
#include "types.hpp"

class core {
public:
    struct WindowSize {
        int32 width; // in pixels (size of framebuffer)
        int32 height; // in pixels (size of framebuffer)
        uint32 screencoord_width; // in screen coordinates
        uint32 screencoord_height; // in screen coordinates
    };

    static int run();

private:
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
    static void mouse_callback(GLFWwindow *window, real64 x, real64 y);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void char_callback(GLFWwindow* window, uint32 codepoint);
    static GLFWwindow * init_window(WindowSize *window_size);
    static bool init_state(State *state, MemoryPool *asset_memory_pool);
    static void destroy_state(State *state);
};
