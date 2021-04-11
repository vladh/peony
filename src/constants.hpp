#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

constexpr real32 PI32 = 3.14159265359f;
constexpr real64 PI = 3.14159265358979323846;

#if !defined(MAX_PATH)
#if defined(PATH_MAX)

#define MAX_PATH PATH_MAX

#else

#define MAX_PATH 260

#endif
#endif

constexpr uint32 MAX_N_ENTITIES = 256;
constexpr uint32 MAX_N_MODELS = 128;
constexpr uint32 MAX_N_ANIMATED_MODELS = 128;
constexpr uint32 MAX_DEBUG_NAME_LENGTH = 256;
constexpr uint32 MAX_GENEROUS_STRING_LENGTH = 512;
constexpr uint32 MAX_N_MESHES = 128;
constexpr uint32 MAX_N_MATERIALS = 256;
constexpr uint32 MAX_N_MATERIALS_PER_MODEL = 16;
constexpr uint32 MAX_N_TEXTURES_PER_MATERIAL = 16;
constexpr uint32 MAX_TOKEN_LENGTH = 128;
constexpr uint32 MAX_N_BONES = 128;
constexpr uint32 MAX_N_BONES_PER_VERTEX = 4;
constexpr uint32 MAX_NODE_NAME_LENGTH = 32;
constexpr uint32 MAX_N_ANIMATIONS = 2;
constexpr uint32 MAX_N_ANIM_KEYS = 256;

#endif
