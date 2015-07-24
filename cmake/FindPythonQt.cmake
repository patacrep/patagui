# Find PythonQt
#
# Sets PYTHONQT_FOUND, PYTHONQT_INCLUDE_DIR, PYTHONQT_LIBRARY, PYTHONQT_LIBRARIES
#

include(ExternalProject)

ExternalProject_Add(PythonQt
PREFIX pythonqt-build
URL http://sourceforge.net/projects/pythonqt/files/pythonqt/PythonQt-3.0/PythonQt3.0.zip
PATCH_COMMAND patch build/python.prf < ${CMAKE_CURRENT_SOURCE_DIR}/cmake/python34.patch
CONFIGURE_COMMAND qmake ${CMAKE_CURRENT_BINARY_DIR}/pythonqt-build/src/PythonQt/PythonQt.pro
BUILD_COMMAND make
)

set(PYTHONQT_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/pythonqt-build/src/PythonQt/)
find_path(PYTHONQT_INCLUDE_DIR PythonQt.h "${PYTHONQT_INSTALL_DIR}/src" DOC "Path to the PythonQt include directory")
find_library(PYTHONQT_LIBRARY PythonQt PATHS "${CMAKE_CURRENT_BINARY_DIR}/pythonqt-build/src/PythonQt-build/lib" DOC "The PythonQt library.")

message(STATUS ${PYTHONQT_INSTALL_DIR})
message(STATUS ${PYTHONQT_INCLUDE_DIR})
message(STATUS ${PYTHONQT_LIBRARY})

mark_as_advanced(PYTHONQT_INSTALL_DIR)
mark_as_advanced(PYTHONQT_INCLUDE_DIR)
mark_as_advanced(PYTHONQT_LIBRARY)

# On linux, also find libutil
if(UNIX AND NOT APPLE)
  find_library(PYTHONQT_LIBUTIL util)
  mark_as_advanced(PYTHONQT_LIBUTIL)
endif()

set(PYTHONQT_FOUND 0)
if(PYTHONQT_INCLUDE_DIR AND PYTHONQT_LIBRARY)
  # Currently CMake'ified PythonQt only supports building against a python Release build.
  # This applies independently of CTK build type (Release, Debug, ...)
  add_definitions(-DPYTHONQT_USE_RELEASE_PYTHON_FALLBACK)
  set(PYTHONQT_FOUND 1)
  set(PYTHONQT_LIBRARIES ${PYTHONQT_LIBRARY} ${PYTHONQT_LIBUTIL})
endif()

if(NOT ${PYTHONQT_FOUND})
  message(FATAL_ERROR "PythonQt is required to build ${PROJECT_NAME}")
else()
  message(STATUS "PythonQt found")
endif()
