#ifndef GL_H
#define GL_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdio.h>

#include <set> // Remove?
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/quaternion.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>
#include "glad.cpp"

#include "types.hpp"
#include "render.hpp"
#include "memory.hpp"
#include "array.hpp"
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
