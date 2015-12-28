# Find PythonQt
#
# Sets PYTHONQT_FOUND, PYTHONQT_INCLUDE_DIR, PYTHONQT_LIBRARY, PYTHONQT_LIBRARIES
#

include(ExternalProject)


# Extract PythonQt from modified sources, then build
ExternalProject_Add(Yaml-cpp
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

