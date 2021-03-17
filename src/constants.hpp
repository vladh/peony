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

constexpr uint16 MAX_DEBUG_NAME_LENGTH = 256;
constexpr uint16 MAX_GENEROUS_STRING_LENGTH = 512;

#endif
