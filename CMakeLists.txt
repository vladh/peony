cmake_minimum_required(VERSION 3.16.0)
project(peony VERSION 1.0.0 LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

add_executable(
  peony
  # WIN32
  src/peony.cc
)

find_package(glfw REQUIRED)
find_package(freetype REQUIRED)
find_package(assimp REQUIRED)
target_link_libraries(peony PRIVATE glfw freetype assimp)

if (MSVC)
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
