// (c) 2020 Vlad-Stefan Harbuz <vlad@vladh.net>

#pragma once

#include "types.hpp"

#if defined(PLATFORM_UNIX)
#include <unistd.h>
#include <limits.h>
#endif

// Math things
constexpr f32 PI32 = 3.14159265359f;
constexpr f64 PI = 3.14159265358979323846;

// Platform things
#if !defined(MAX_PATH)
#if defined(PATH_MAX)
#define MAX_PATH PATH_MAX
#else
#define MAX_PATH 260
#endif
#endif

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || \
    defined(__NT__) || defined(__CYGWIN__) || defined(__MINGW32__)
#define PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM_MACOS
#define PLATFORM_POSIX
#elif defined(__linux__) || defined(__unix__)
#define PLATFORM_UNIX
#define PLATFORM_POSIX
#elif defined(__FreeBSD)
#define PLATFORM_FREEBSD
#define PLATFORM_POSIX
#else
#error "Unknown platform"
#endif

#define USE_TIMERS true
#define USE_MEMORYPOOL_ITEM_DEBUG false

enum class GraphicsQuality { low, high };

struct Settings {
    GraphicsQuality graphics_quality;
    bool opengl_debug_on;
    bool vsync_on;
    bool shader_debug_on;
    bool shadows_on;
    bool bloom_on;
    bool fog_on;
    bool windowed_fullscreen_on;
    bool fullscreen_on;
    u32 target_monitor;
    bool print_fps_on;
    bool memory_debug_logs_on;
};

Settings SETTINGS = {};

constexpr char WINDOW_TITLE[] = "peony";
constexpr char TEXTURE_DIR[] = "resources/textures/";
constexpr char MODEL_DIR[] = "resources/models/";
constexpr char FONTS_DIR[] = "resources/fonts/";
constexpr char SCENE_DIR[] = "data/scenes/";
constexpr char MATERIAL_FILE_DIRECTORY[] = "data/materials/";
constexpr char SHADER_COMMON_PATH[] = "src_shaders/common.glsl";
constexpr char SHADER_COMMON_FRAGMENT_PATH[] = "src_shaders/common_fragment.glsl";
constexpr char SHADER_DIR[] = "src_shaders/";
constexpr char DEFAULT_SCENE[] = "terraintest";
constexpr char SCENE_EXTENSION[] = ".peony_scene";
constexpr char MATERIAL_FILE_EXTENSION[] = ".peony_materials";
constexpr u32 N_LOADING_THREADS = 5;
constexpr u32 MAX_N_ENTITIES = 256;
constexpr u32 MAX_N_MODELS = 128;
constexpr u32 MAX_N_ANIMATED_MODELS = 128;
constexpr u32 MAX_DEBUG_NAME_LENGTH = 256;
constexpr u32 MAX_GENEROUS_STRING_LENGTH = 512;
constexpr u32 MAX_N_MESHES = 128;
constexpr u32 MAX_N_MATERIALS = 256;
constexpr u32 MAX_N_MATERIALS_PER_MODEL = 16;
constexpr u32 MAX_UNIFORM_LENGTH = 256;
constexpr u32 MAX_N_TEXTURE_POOL_SIZES = 6;
constexpr u32 MAX_N_TEXTURES_PER_MATERIAL = 16;
constexpr u8 MAX_N_UNIFORMS = 64;
constexpr u8 MAX_UNIFORM_NAME_LENGTH = 64;
constexpr u8 MAX_N_TEXTURE_UNITS = 80;
constexpr u32 MAX_COMMON_NAME_LENGTH = 128;
constexpr u32 MAX_N_BONES = 128;
constexpr u32 MAX_N_BONES_PER_VERTEX = 4;
constexpr u32 MAX_NODE_NAME_LENGTH = 32;
constexpr u32 MAX_N_ANIMATIONS = 2;
constexpr u32 MAX_N_ANIM_KEYS = 256;
constexpr u16 MAX_N_LIGHTS = 8;


void
init_constants()
{
#if defined(PLATFORM_UNIX)
    char hostname[HOST_NAME_MAX];
    gethostname(hostname, HOST_NAME_MAX);
#else
    char *hostname = "dunno lol!";
#endif

    if (pstr_eq(hostname, "usu")) {
        SETTINGS = {
            .graphics_quality = GraphicsQuality::low,
            .opengl_debug_on = false,
            .vsync_on = false,
            .shader_debug_on = false,
            .shadows_on = true,
            .bloom_on = true,
            .fog_on = false,
            .windowed_fullscreen_on = true,
            .fullscreen_on = false,
            .target_monitor = 0,
            .print_fps_on = false,
            .memory_debug_logs_on = false,
        };
    } else {
        SETTINGS = {
            .graphics_quality = GraphicsQuality::high,
            .opengl_debug_on = false,
            .vsync_on = false,
            .shader_debug_on = false,
            .shadows_on = true,
            .bloom_on = true,
            .fog_on = false,
            .windowed_fullscreen_on = true,
            .fullscreen_on = true,
            .target_monitor = 0,
            .print_fps_on = false,
            .memory_debug_logs_on = false,
        };
    }
}
