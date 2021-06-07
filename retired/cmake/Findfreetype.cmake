find_library(
  FREETYPE_LIBRARY
  NAMES freetype
  HINTS
  C:/local/lib
  ~/local/lib
  /usr/local/opt/freetype/lib
)
find_path(
  FREETYPE_INCLUDE_DIR
  NAMES freetype
  HINTS
  C:/local/include
  ~/local/include
  /usr/local/opt/freetype/include
  /usr/local/opt/freetype/include/freetype2
)

include(FindPackageHandleStandardArgs)

find_package_handle_standard_args(
  freetype
  DEFAULT_MSG
  FREETYPE_LIBRARY
  FREETYPE_INCLUDE_DIR
)

if(FREETYPE_FOUND)
  add_library(freetype SHARED IMPORTED)
  set_target_properties(
    freetype
    PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${FREETYPE_INCLUDE_DIR}"
    IMPORTED_LOCATION ${FREETYPE_LIBRARY} # .dll
    IMPORTED_IMPLIB ${FREETYPE_LIBRARY} # .lib
  )
  message(STATUS "FREETYPE_INCLUDE_DIR = ${FREETYPE_INCLUDE_DIR}")
  message(STATUS "FREETYPE_LIBRARY = ${FREETYPE_LIBRARY}")
else()
  message(STATUS "freetype not found")
endif()
