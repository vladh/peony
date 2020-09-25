#ifndef GL_H
#define GL_H

#define SHADER_DIR "src/shaders/"
#define SHADER_COMMON_PATH SHADER_DIR"common.glsl"

// TODO: Decide which ones we should actually be including.
// Add assert.h?
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>

#if defined(__unix__)
#include <unistd.h>
#endif

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

// TODO: Remove set?
#include <set>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/string_cast.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#if USE_OPENGL_4
#include "glad/glad_430.cpp"
#else
#include "glad/glad_330.cpp"
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

#include "types.hpp"
#include "pack.hpp"
#include "render.hpp"
#include "memory.hpp"
#include "array.hpp"
#include "font.hpp"
#include "shader.hpp"
#include "models.hpp"
#include "entity.hpp"
#include "data.hpp"
#include "asset.hpp"
#include "control.hpp"
#include "log.hpp"
#include "util.hpp"
#include "camera.hpp"
#include "state.hpp"

#endif
