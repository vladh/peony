#ifndef TYPES_H
#define TYPES_H

#define global_variable static
#define local_persist static
#define internal static

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef int32 bool32;
typedef float real32;
typedef double real64;

#define PI 3.14159265358979323846

#define KILOBYTES(value) ((value) * 1024LL)
#define MEGABYTES(value) (KILOBYTES(value) * 1024LL)
#define GIGABYTES(value) (MEGABYTES(value) * 1024LL)
#define TERABYTES(value) (GIGABYTES(value) * 1024LL)
#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)
#define LEN(x) (sizeof(x) / sizeof((x)[0]))
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#endif
