#ifndef PEONY_HPP
#define PEONY_HPP

#define _HAS_EXCEPTIONS 0

#if USE_VLD
#include <vld.h>
#endif

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

#include <chrono>
namespace chrono = std::chrono;
#include <thread>
#include <mutex>

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
#define STB_IMAGE_IMPLEMENTATION
#pragma warning(push, 0)
#pragma warning(disable: 6011)
#pragma warning(disable: 6308)
#pragma warning(disable: 6262)
#pragma warning(disable: 28182)
#include <stb/stb_image.h>
#pragma warning(pop)
#if 0
#define TSL_NO_EXCEPTIONS
#include <tsl/robin_map.h>
#include <tsl/robin_set.h>
#endif

#define NOMINMAX
#include "../include/glad.cpp"

#include <ft2build.h>
#include FT_FREETYPE_H

#if USE_CACHELINE_SIZE_DISPLAY
#include "../include/cacheline.hpp"
#endif

#include "types.hpp"
#include "constants.hpp"
#include "intrinsics.hpp"
#include "debug.hpp"
#include "log.hpp"
#include "memory.hpp"
#include "util.hpp"
#include "str.hpp"
#include "pack.hpp"
#include "queue.hpp"
#include "array.hpp"
#include "stackarray.hpp"
#include "shaders.hpp"
#include "materials.hpp"
#include "tasks.hpp"
#include "fonts.hpp"
#include "renderer.hpp"
#include "physics.hpp"
#include "entities.hpp"
#include "debugdraw.hpp"
#include "entity_sets.hpp"
#include "models.hpp"
#include "peony_file_parser.hpp"
#include "input.hpp"
#include "gui.hpp"
#include "data.hpp"
#include "camera.hpp"
#include "state.hpp"
#include "world.hpp"

#endif
