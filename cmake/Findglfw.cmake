find_library(
  GLFW_LIBRARY
  NAMES glfw3 libglfw.dylib
  HINTS
  C:/local/lib
  ~/local/lib
  /usr/local/opt/glfw/lib
)
find_path(
  GLFW_INCLUDE_DIR
  NAMES GLFW
  HINTS
  C:/local/lib
  ~/local/include
  /usr/local/opt/glfw/include
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  glfw
  DEFAULT_MSG
  GLFW_LIBRARY
  GLFW_INCLUDE_DIR
)

if(GLFW_FOUND)
  add_library(glfw SHARED IMPORTED)
  set_target_properties(
    glfw
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIR}"
    IMPORTED_LOCATION ${GLFW_LIBRARY} # .dll
    IMPORTED_IMPLIB ${GLFW_LIBRARY} # .lib
  )
  message(STATUS "GLFW_INCLUDE_DIR = ${GLFW_INCLUDE_DIR}")
  message(STATUS "GLFW_LIBRARY = ${GLFW_LIBRARY}")
else()
  message(STATUS "glfw not found")
endif()
