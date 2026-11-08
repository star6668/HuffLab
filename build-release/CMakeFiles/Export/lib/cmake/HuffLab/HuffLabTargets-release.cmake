#----------------------------------------------------------------
# Generated CMake target import file for configuration "Release".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "HuffLab::hufflab" for configuration "Release"
set_property(TARGET HuffLab::hufflab APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
set_target_properties(HuffLab::hufflab PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX"
  IMPORTED_LOCATION_RELEASE "${_IMPORT_PREFIX}/lib/libhufflab_core.a"
  )

list(APPEND _IMPORT_CHECK_TARGETS HuffLab::hufflab )
list(APPEND _IMPORT_CHECK_FILES_FOR_HuffLab::hufflab "${_IMPORT_PREFIX}/lib/libhufflab_core.a" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
