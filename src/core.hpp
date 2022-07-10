// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include <GLFW/glfw3.h>
#include "types.hpp"

class core {
public:
    static int run();
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
    static void mouse_callback(GLFWwindow *window, real64 x, real64 y);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void char_callback(GLFWwindow* window, uint32 codepoint);

private:
    static bool init_state(State *state, MemoryPool *asset_memory_pool);
    static void destroy_state(State *state);
};
