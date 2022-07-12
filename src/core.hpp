// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include <GLFW/glfw3.h>
#include "types.hpp"
#include "renderer.hpp"
#include "common.hpp"

class core {
public:
    static WindowSize * get_window_size();
    static int run();
    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    static void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
    static void mouse_callback(GLFWwindow *window, f64 x, f64 y);
    static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void char_callback(GLFWwindow* window, u32 codepoint);

private:
    static bool init_state(State *state, memory::Pool *asset_memory_pool);
    static memory::Pool * get_asset_memory_pool();
    static void destroy_state(State *state);

    static State *state;
};
