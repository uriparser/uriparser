# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

#.rst:
# FindURIPARSER
# --------
# This is adapted from the FindZLIB script included with CMake.
#
# Find the native URIPARSER includes and library.
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# This module defines :prop_tgt:`IMPORTED` target ``URIPARSER``, if
# URIPARSER has been found.
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables:
#
# ::
#
#   URIPARSER_INCLUDE_DIRS   - where to find URIPARSER includes
#   URIPARSER_LIBRARIES      - List of libraries when using URIPARSER
#   URIPARSER_FOUND          - True if URIPARSER found.
#
# ::
#
#   URIPARSER_VERSION_STRING - The version of URIPARSER found (x.y.z)
#   URIPARSER_VERSION_MAJOR  - The major version of URIPARSER
#   URIPARSER_VERSION_MINOR  - The minor version of URIPARSER
#   URIPARSER_VERSION_PATCH  - The patch version of URIPARSER
#
# Hints
# ^^^^^
#
# A user may set ``URIPARSER_ROOT`` to a URIPARSER installation root to tell this
# module where to look.

include(SelectLibraryConfigurations)
include(FindPackageHandleStandardArgs)

set(URIPARSER_NAMES uriparser liburiparser)

find_path(URIPARSER_INCLUDE_DIR
  NAMES uriparser/UriBase.h
  PATH_SUFFIXES include)

find_library(URIPARSER_LIBRARY_RELEASE
  NAMES ${URIPARSER_NAMES}
  NAMES_PER_DIR
  PATH_SUFFIXES lib)

select_library_configurations(URIPARSER)

mark_as_advanced(URIPARSER_INCLUDE_DIR)

if(URIPARSER_INCLUDE_DIR)
    file(READ "${URIPARSER_INCLUDE_DIR}/uriparser/UriBase.h" URI_H)

    string(REGEX REPLACE "#define[ \t]+URI_VER_MAJOR[ \t]+([0-9]+)"
      VERSION_REGEX_MATCH ${URI_H})

    set(URIPARSER_MAJOR_VERSION ${CMAKE_MATCH_1})

    string(REGEX MATCH "#define[ \t]+URI_VER_MINOR[ \t]+([0-9]+)"
      VERSION_REGEX_MATCH ${URI_H})

    set(URIPARSER_MINOR_VERSION ${CMAKE_MATCH_1})

    string(REGEX MATCH "#define[ \t]+URI_VER_RELEASE[ \t]+([0-9]+)"
      VERSION_REGEX_MATCH ${URI_H})

    set(URIPARSER_PATCH_VERSION ${CMAKE_MATCH_1})

    set(URIPARSER_VERSION_STRING ${URIPARSER_MAJOR_VERSION}.${URIPARSER_MINOR_VERSION}.${URIPARSER_PATCH_VERSION})
endif()

find_package_handle_standard_args(URIPARSER
  REQUIRED_VARS URIPARSER_LIBRARY URIPARSER_INCLUDE_DIR
  VERSION_VAR URIPARSER_VERSION_STRING)

if(URIPARSER_FOUND)
    set(URIPARSER_INCLUDE_DIRS ${URIPARSER_INCLUDE_DIR})

    if(NOT URIPARSER_LIBRARIES)
      set(URIPARSER_LIBRARIES ${URIPARSER_LIBRARY})
    endif()

    if(NOT TARGET uriparser)
      add_library(uriparser UNKNOWN IMPORTED)
      set_target_properties(uriparser PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${URIPARSER_INCLUDE_DIRS}")

      if(URIPARSER_LIBRARY_RELEASE)
        set_property(TARGET uriparser APPEND PROPERTY
          IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(uriparser PROPERTIES
          IMPORTED_LOCATION_RELEASE "${URIPARSER_LIBRARY_RELEASE}")
      endif()

      if(URIPARSER_LIBRARY_DEBUG)
        set_property(TARGET uriparser APPEND PROPERTY
          IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(uriparser PROPERTIES
          IMPORTED_LOCATION_DEBUG "${URIPARSER_LIBRARY_DEBUG}")
      endif()

      if(NOT URIPARSER_LIBRARY_RELEASE AND NOT URIPARSER_LIBRARY_DEBUG)
        set_property(TARGET uriparser APPEND PROPERTY
          IMPORTED_LOCATION "${URIPARSER_LIBRARY}")
      endif()
    endif()
endif()