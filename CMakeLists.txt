#-------------------------------------------------------------------------------
project(songbook-client)
#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.6)
#-------------------------------------------------------------------------------
# process options and parameters
if(DEFINED PREFIX)
  set(CMAKE_INSTALL_PREFIX ${PREFIX})
endif()
#-------------------------------------------------------------------------------
find_package(Qt4 COMPONENTS QtCore QtGui QtSql REQUIRED)
set(QT_USE_QTSQL true)
#-------------------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
message(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
#-------------------------------------------------------------------------------
# sources
set(SONGBOOK_CLIENT_SOURCES
  src/main.cc
  src/mainwindow.cc
  src/preferences.cc
  src/library.cc
  src/songbook.cc
  src/header.cc
  src/tools.cc
  src/download.cc
  src/custom.cc
  )
#-------------------------------------------------------------------------------
# header (moc)
set(SONGBOOK_CLIENT_QT_HEADER
  src/mainwindow.hh
  src/preferences.hh
  src/library.hh
  src/tools.hh
  src/download.hh
  src/custom.hh
  )
#-------------------------------------------------------------------------------
# ressources
set(SONGBOOK_CLIENT_RCS
  songbook.qrc
  )
#-------------------------------------------------------------------------------
# process Qt special files
include(${QT_USE_FILE})
qt4_wrap_cpp(SONGBOOK_CLIENT_MOCS ${SONGBOOK_CLIENT_QT_HEADER})
qt4_add_resources(SONGBOOK_CLIENT_RESSOURCES ${SONGBOOK_CLIENT_RCS})
include_directories (${CMAKE_CURRENT_BINARY_DIR} src)
#-------------------------------------------------------------------------------
#internationalization
SET (TRANSLATIONS
lang/songbook_en.ts
lang/songbook_fr.ts)

SET (COMPILED_TRANSLATIONS
lang/songbook_fr.qm
lang/songbook_en.qm)

ADD_CUSTOM_COMMAND (OUTPUT
${COMPILED_TRANSLATIONS}
COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
DEPENDS ${TRANSLATIONS}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
#-------------------------------------------------------------------------------
# generating executable
add_executable(songbook-client
  ${SONGBOOK_CLIENT_SOURCES} 
  ${SONGBOOK_CLIENT_MOCS} 
  ${SONGBOOK_CLIENT_RESSOURCES} 
  ${COMPILED_TRANSLATIONS}
)
target_link_libraries(songbook-client ${QT_LIBRARIES})
#-------------------------------------------------------------------------------
# install instructions
install(TARGETS songbook-client RUNTIME DESTINATION bin)
install(FILES songbook-client.desktop DESTINATION share/applications)
install(FILES icons/patacrep.png DESTINATION share/icons)
install(FILES ${COMPILED_TRANSLATIONS} DESTINATION share/songbook-client/translations)
