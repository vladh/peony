cmake_minimum_required(VERSION 3.16.0)
project(peony VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
set(UNITY false)

# Treat all .c files as .cpp files
file(GLOB_RECURSE CFILES "${CMAKE_SOURCE_DIR}/src_external/*.c")
set_source_files_properties(${CFILES} PROPERTIES LANGUAGE CXX)

if (UNITY)
  add_executable(
    peony
    # WIN32
    src/_unity.cpp
  )
else()
  add_executable(
    peony
    # WIN32
    src_external/glad.cpp
    src_external/pstr.c
    src/logs.cpp
    src/util.cpp
    src/debug.cpp
    src/memory.cpp
    src/files.cpp
    src/pack.cpp
    src/shaders.cpp
    src/tasks.cpp
    src/materials.cpp
    src/fonts.cpp
    src/entities.cpp
    src/spatial.cpp
    src/lights.cpp
    src/behavior.cpp
    src/anim.cpp
    src/debugdraw.cpp
    src/input.cpp
    src/gui.cpp
    src/physics.cpp
    src/models.cpp
    src/peony_parser.cpp
    src/peony_parser_utils.cpp
    src/cameras.cpp
    src/debug_ui.cpp
    src/renderer.cpp
    src/internals.cpp
    src/engine.cpp
    src/behavior_functions.cpp
    src/core.cpp
    src/main.cpp
  )
endif()

find_package(glfw REQUIRED)
find_package(freetype REQUIRED)
find_package(assimp REQUIRED)
find_package(glm REQUIRED)
target_link_libraries(peony PRIVATE glfw freetype assimp glm)
target_precompile_headers(peony PRIVATE src/flags.hpp)

if (MSVC)
  set_property(
    TARGET peony PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  )
  add_compile_options(
    -FC -GS -MTd -MP18 -W4 -WL -WX -Z7 -Zo -diagnostics:column -fp:fast -nologo
    -sdl -guard:cf -permissive- -std:c++latest -wd4100 -wd4127 -wd4201 -O2 -GL -Oi
  )
  add_link_options(
    -CGTHREADS:8 -DEBUG:FULL -INCREMENTAL:NO -opt:ref
    opengl32.lib user32.lib gdi32.lib shell32.lib
  )
else()
  add_compile_options(
    -g3 -D_FORTIFY_SOURCE=2 -O2 -Wall -Werror -Wno-deprecated-volatile -pedantic
  )
  # TODO
endif()
