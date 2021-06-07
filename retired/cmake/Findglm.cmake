find_path(
  GLM_INCLUDE_DIR
  NAMES glm
  HINTS
  C:/local/lib
  ~/local/include
  /usr/local/opt/glm/include
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  glm
  DEFAULT_MSG
  GLM_INCLUDE_DIR
)

if(GLM_FOUND)
  add_library(glm INTERFACE IMPORTED)
  set_target_properties(
    glm
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GLM_INCLUDE_DIR}"
  )
  message(STATUS "GLM_INCLUDE_DIR = ${GLM_INCLUDE_DIR}")
else()
  message(STATUS "glm not found")
endif()
