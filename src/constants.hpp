#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

// Options

#define USE_TIMERS true
#define USE_MEMORY_DEBUG_LOGS false
#define USE_MEMORYPOOL_ITEM_DEBUG false
#define USE_FULLSCREEN true
#define USE_WINDOWED_FULLSCREEN true
#define USE_ANIMATION_DEBUG false
#define USE_SHADER_DEBUG false
#define USE_BLOOM false
#define TARGET_MONITOR 0
#define USE_OPENGL_DEBUG false

constexpr real32 PI32 = 3.14159265359f;
constexpr real64 PI = 3.14159265358979323846;

#if !defined(MAX_PATH)
#if defined(PATH_MAX)

#define MAX_PATH PATH_MAX

#else

#define MAX_PATH 260

#endif
#endif

constexpr char TEXTURE_DIR[] = "resources/textures/";
constexpr char MODEL_DIR[] = "resources/models/";
constexpr char FONTS_DIR[] = "resources/fonts/";
constexpr char SCENE_DIR[] = "data/scenes/";
constexpr char MATERIAL_FILE_DIRECTORY[] = "data/materials/";
constexpr char SHADER_COMMON_PATH[] = "shaders/common.glsl";
constexpr char SHADER_COMMON_FRAGMENT_PATH[] = "shaders/common_fragment.glsl";
constexpr char SHADER_DIR[] = "shaders/";
constexpr char DEFAULT_SCENE[] = "terraintest";
constexpr char SCENE_EXTENSION[] = ".peony_scene";
constexpr char MATERIAL_FILE_EXTENSION[] = ".peony_materials";
constexpr uint32 MAX_N_ENTITIES = 256;
constexpr uint32 MAX_N_MODELS = 128;
constexpr uint32 MAX_N_ANIMATED_MODELS = 128;
constexpr uint32 MAX_DEBUG_NAME_LENGTH = 256;
constexpr uint32 MAX_GENEROUS_STRING_LENGTH = 512;
constexpr uint32 MAX_N_MESHES = 128;
constexpr uint32 MAX_N_MATERIALS = 256;
constexpr uint32 MAX_N_MATERIALS_PER_MODEL = 16;
constexpr uint32 MAX_UNIFORM_LENGTH = 256;
constexpr uint32 MAX_N_TEXTURE_POOL_SIZES = 6;
constexpr uint32 MAX_N_TEXTURES_PER_MATERIAL = 16;
constexpr uint8 MAX_N_UNIFORMS = 64;
constexpr uint8 MAX_UNIFORM_NAME_LENGTH = 64;
constexpr uint8 MAX_N_TEXTURE_UNITS = 80;
constexpr uint32 MAX_TOKEN_LENGTH = 128;
constexpr uint32 MAX_N_BONES = 128;
constexpr uint32 MAX_N_BONES_PER_VERTEX = 4;
constexpr uint32 MAX_NODE_NAME_LENGTH = 32;
constexpr uint32 MAX_N_ANIMATIONS = 2;
constexpr uint32 MAX_N_ANIM_KEYS = 256;
constexpr uint32 MAX_N_PEONY_ARRAY_VALUES = 16;
constexpr uint16 MAX_N_LIGHTS = 8;

#endif
