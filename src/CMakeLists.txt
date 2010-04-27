#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.6.2)
project (songbook)
find_package(Qt4 REQUIRED)

set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall")
set(CMAKE_CXX_FLAGS_RELEASE "-DINLINE_FLAG -O2 -finline-functions -DNDEBUG -march=native")

#-------------------------------------------------------------------------------
set (IHM_SRCS 
  main.cc
  mainwindow.cc
  preferences.cc
  library.cc
  songbook.cc
  header.cc
  )

set (NEED_MOCS 
  mainwindow.hh
  preferences.hh
  library.hh
  )

set (IHM_RCS
  application.qrc
  )
#-------------------------------------------------------------------------------
include(${QT_USE_FILE})
include_directories( ${QT_QTSQL_INCLUDE_DIR})
qt4_wrap_cpp(MOC_SRCS ${NEED_MOCS})
qt4_add_resources( IHM_RC_RCS ${IHM_RCS} )
#-------------------------------------------------------------------------------
add_executable(./songbook ${IHM_SRCS} ${MOC_SRCS} ${IHM_RC_RCS})
target_link_libraries(./songbook ${QT_LIBRARIES})
target_link_libraries(./songbook ${QT_QTSQL_LIBRARY})
#-------------------------------------------------------------------------------
