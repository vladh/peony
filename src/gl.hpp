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

#include <chrono>
#include <thread>
#include <mutex>

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
#include "../include/glad_430.cpp"
#else
#include "../include/glad_330.cpp"
#endif

#include <ft2build.h>
#include FT_FREETYPE_H

#include "../include/cacheline.hpp"

#include "types.hpp"
#include "log.hpp"
#include "memory.hpp"
#include "util.hpp"
#include "pack.hpp"
#include "render.hpp"
#include "queue.hpp"
#include "array.hpp"
#include "resource_manager.hpp"
#include "asset.hpp"
#include "font_asset.hpp"
#include "shader_asset.hpp"
#include "persistent_pbo.hpp"
#include "texture_set_asset.hpp"
#include "model_asset.hpp"
#include "entity.hpp"
#include "component.hpp"
#include "entity_manager.hpp"
#include "drawable_component_manager.hpp"
#include "light_component_manager.hpp"
#include "spatial_component_manager.hpp"
#include "text_manager.hpp"
#include "data.hpp"
#include "control.hpp"
#include "camera.hpp"
#include "state.hpp"

#endif
