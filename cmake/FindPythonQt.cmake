# Find PythonQt
#
# Sets PYTHONQT_FOUND, PYTHONQT_INCLUDE_DIR, PYTHONQT_LIBRARY, PYTHONQT_LIBRARIES
#

include(ExternalProject)

ExternalProject_Add(PythonQt
PREFIX pythonqt
SVN_REPOSITORY svn://svn.code.sf.net/p/pythonqt/code/trunk
SVN_REVISION 403
# URL "http://downloads.sourceforge.net/project/pythonqt/pythonqt/PythonQt-3.0/PythonQt3.0.zip"
PATCH_COMMAND patch -p0 --ignore-whitespace -i ${CMAKE_CURRENT_SOURCE_DIR}/cmake/python3.patch
CONFIGURE_COMMAND qmake ${CMAKE_CURRENT_BINARY_DIR}/pythonqt/src/PythonQt/PythonQt.pro
BUILD_COMMAND make
)

set(PYTHONQT_INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/pythonqt/src/PythonQt)
set(PYTHONQT_INCLUDE_DIR ${PYTHONQT_INSTALL_DIR}/src)
set(PYTHONQT_LIBRARY ${CMAKE_CURRENT_BINARY_DIR}/pythonqt/src/PythonQt-build/lib)

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
