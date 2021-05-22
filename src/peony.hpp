#pragma once

// Flags
#define _HAS_EXCEPTIONS 0
#define _CRT_SECURE_NO_WARNINGS
#define NOMINMAX
#pragma warning(disable: 4505) // unreferenced local function has been removed

// C stuff
#include <float.h>
#include <math.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <intrin.h>
#endif

// C++ stuff
#include <chrono>
namespace chrono = std::chrono;
#include <thread>
#include <mutex>

// Libraries
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
using glm::dot, glm::cross, glm::normalize, glm::abs, glm::max, glm::min,
  glm::ceil, glm::floor, glm::degrees, glm::radians, glm::transpose,
  glm::inverse, glm::length, glm::length2;

#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

/* #include <vld.h> */

#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push, 0)
#pragma warning(disable: 6011)
#pragma warning(disable: 6308)
#pragma warning(disable: 6262)
#pragma warning(disable: 28182)
#include <stb/stb_image.h>
#pragma warning(pop)

#include "../include/glad.cpp"

#include <ft2build.h>
#include FT_FREETYPE_H

// Our headers
#include "intrinsics.hpp"
#include "types.hpp"
#include "constants.hpp"
#include "debug.hpp"
#include "logs.hpp"
#include "util.hpp"
#include "memory.hpp"
#include "files.hpp"
#include "str.hpp"
#include "pack.hpp"
#include "queue.hpp"
#include "array.hpp"
#include "stackarray.hpp"
#include "shaders.hpp"
#include "tasks.hpp"
#include "materials.hpp"
#include "fonts.hpp"
#include "entities.hpp"
#include "spatial.hpp"
#include "lights.hpp"
#include "behavior.hpp"
#include "anim.hpp"
#include "debugdraw.hpp"
#include "input.hpp"
#include "gui.hpp"
#include "physics.hpp"
#include "models.hpp"
#include "peony_parser.hpp"
#include "cameras.hpp"
#include "state.hpp"
#include "debug_ui.hpp"
#include "renderer.hpp"
#include "internals.hpp"
#include "engine.hpp"
#include "behavior_functions.hpp"
