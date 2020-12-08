#ifndef GL_H
#define GL_H

#define _HAS_EXCEPTIONS 0

#if USE_VLD
#include <vld.h>
#endif

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>

#include <intrin.h>

#include <chrono>
#include <thread>
#include <mutex>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
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
#include "math.hpp"
#include "debug.hpp"
#include "log.hpp"
#include "memory_pool.hpp"
#include "memory.hpp"
#include "util.hpp"
#include "task.hpp"
#include "pack.hpp"
#include "render.hpp"
#include "queue.hpp"
#include "array.hpp"
#include "resource_manager.hpp"
#include "texture_name_pool.hpp"
#include "texture_atlas.hpp"
#include "font_asset.hpp"
#include "shader_asset.hpp"
#include "persistent_pbo.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "entity.hpp"
#include "component.hpp"
#include "spatial_component.hpp"
#include "drawable_component.hpp"
#include "light_component.hpp"
#include "behavior_component.hpp"
#include "entity_manager.hpp"
#include "drawable_component_manager.hpp"
#include "light_component_manager.hpp"
#include "spatial_component_manager.hpp"
#include "behavior_component_manager.hpp"
#include "model_asset.hpp"
#include "input_manager.hpp"
#include "gui_manager.hpp"
#include "data.hpp"
#include "camera.hpp"
#include "state.hpp"

#endif