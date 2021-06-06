cmake_minimum_required(VERSION 3.16.0)
project(peony VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(UNITY true)

# Treat all .c files as .cpp files
file(GLOB_RECURSE CFILES "${CMAKE_SOURCE_DIR}/src_external/*.c")
set_source_files_properties(${CFILES} PROPERTIES LANGUAGE CXX)

# Get source files
if (UNITY)
  set(SOURCE_FILES "src/_unity.cpp")
else()
  file(
    GLOB_RECURSE
    SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/src_external/*"
    "${CMAKE_SOURCE_DIR}/src/*"
  )
endif()

# Set the bundle type
set(BUNDLE_TYPE "")
if (MSVC)
  # set(BUNDLE_TYPE "WIN32")
elseif(APPLE)
  set(BUNDLE_TYPE "MACOSX_BUNDLE")
endif()

# Add options (this should be done before `add_executable`)
if (MSVC)
  # Remove absolutely stupid CMake default options
  string(REGEX
    REPLACE "/RTC(su|[1su])" ""
    CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}"
  )
  add_compile_options(
    -FC -GS -MTd -MP18 -W4 -WL -WX -Z7 -Zo -diagnostics:column -fp:fast -nologo
    -sdl -guard:cf -permissive- -std:c++latest
    -GL -Oi -O2
    -wd4100 -wd4127 -wd4201 -wd4505 -wd4706 -wd4702
  )
  add_link_options(
    -CGTHREADS:8 -DEBUG:FULL -INCREMENTAL:NO -opt:ref
    opengl32.lib user32.lib gdi32.lib shell32.lib
  )
elseif(APPLE OR UNIX)
  add_compile_options(
    -ggdb3 -D_FORTIFY_SOURCE=2 -Og -Wall -Werror -Wno-deprecated-volatile -pedantic
    -Wno-unused-function -Wno-unknown-pragmas
  )
  add_link_options(
    -lcrt -lc
  )
endif()

add_executable(peony ${BUNDLE_TYPE} ${SOURCE_FILES})

# Platform-specific properties
if (MSVC)
  set_property(
    TARGET peony PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  )
elseif (APPLE)
  set_target_properties(peony PROPERTIES
    BUNDLE True
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/Info.plist
  )
endif()

find_package(glfw REQUIRED)
find_package(freetype REQUIRED)
find_package(assimp REQUIRED)
find_package(glm REQUIRED)
target_link_libraries(peony PRIVATE glfw freetype assimp glm)
add_definitions(-D_CRT_SECURE_NO_WARNINGS -DNOMINMAX)
