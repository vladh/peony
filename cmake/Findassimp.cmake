find_library(ASSIMP_LIBRARY NAMES assimp-vc142-mtd)
find_path(ASSIMP_INCLUDE_DIR NAMES assimp)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  assimp
  DEFAULT_MSG
  ASSIMP_LIBRARY
  ASSIMP_INCLUDE_DIR
)

if(ASSIMP_FOUND)
  add_library(assimp SHARED IMPORTED)
  set_target_properties(
    assimp
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${ASSIMP_INCLUDE_DIR}"
    IMPORTED_LOCATION ${ASSIMP_LIBRARY} # .dll
    IMPORTED_IMPLIB ${ASSIMP_LIBRARY} # .lib
  )
  message(STATUS "ASSIMP_INCLUDE_DIR = ${ASSIMP_INCLUDE_DIR}")
  message(STATUS "ASSIMP_LIBRARY = ${ASSIMP_LIBRARY}")
else()
  message(STATUS "assimp not found")
endif()
