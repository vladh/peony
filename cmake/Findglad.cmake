find_path(
  GLAD_INCLUDE_DIR
  NAMES glad
  HINTS
  C:/local/lib
  ~/local/include
  /usr/local/opt/glad/include
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  glad
  DEFAULT_MSG
  GLAD_INCLUDE_DIR
)

if(GLAD_FOUND)
  add_library(glad INTERFACE IMPORTED)
  set_target_properties(
    glad
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GLAD_INCLUDE_DIR}"
  )
  message(STATUS "GLAD_INCLUDE_DIR = ${GLAD_INCLUDE_DIR}")
else()
  message(STATUS "glad not found")
endif()
