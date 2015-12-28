# Find Yaml-cpp
#
# Sets YAML-CPP_FOUND, YAML-CPP_INCLUDE_DIR, YAML-CPP_LIBRARY, YAML-CPP_LIBRARIES
#

include(ExternalProject)


# Extract Yaml-cpp from modified sources, then build
ExternalProject_Add(Yaml-cpp-External
PREFIX yaml-cpp
# Backup URL in case SVN messes up.
URL "${CMAKE_CURRENT_SOURCE_DIR}/external_libs/yaml-cpp.zip"
PATCH_COMMAND ""
BUILD_COMMAND ${CMAKE_MAKE_PROGRAM}
BUILD_IN_SOURCE 1
INSTALL_COMMAND ""
# Wrap download, configure and build steps in a script to log output
LOG_DOWNLOAD ON
LOG_CONFIGURE ON
LOG_BUILD ON
)

set(YAML-CPP_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp/src/Yaml-cpp-External)
set(YAML-CPP_INCLUDE_DIR ${YAML-CPP_INSTALL_DIR}/src)
set(YAML-CPP_LIBRARY Yaml-cpp)

# Create dummy Yaml-cpp library that depends on real Yaml-cpp in order to have cmake behaving properly without
# having to resort to a superbuild mechanism.
add_library(Yaml-cpp STATIC IMPORTED)
set_target_properties(Yaml-cpp PROPERTIES IMPORTED_LOCATION ${YAML-CPP_INSTALL_DIR}/${CMAKE_STATIC_LIBRARY_PREFIX}yaml-cpp${CMAKE_STATIC_LIBRARY_SUFFIX})
add_dependencies(Yaml-cpp Yaml-cpp-External)

# Used for OSX deployment.
get_target_property(YAML-CPP_LOCATION Yaml-cpp LOCATION)

mark_as_advanced(YAML-CPP_INSTALL_DIR)
mark_as_advanced(YAML-CPP_INCLUDE_DIR)

set(YAML-CPP_FOUND 0)
if(YAML-CPP_INCLUDE_DIR AND YAML-CPP_LIBRARY)
    set(YAML-CPP_LIBRARIES ${YAML-CPP_LIBRARY})
    set(YAML-CPP_FOUND 1)
endif()

if(NOT ${YAML-CPP_FOUND})
  message(FATAL_ERROR "Yaml-cpp is required to build ${PROJECT_NAME}")
else()
  message(STATUS "Yaml-cpp found")
endif()
