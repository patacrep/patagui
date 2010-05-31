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
  src/song-editor.cc
  src/highlighter.cc
  src/dialog-new-song.cc
  )
#-------------------------------------------------------------------------------
# header (moc)
set(SONGBOOK_CLIENT_QT_HEADER
  src/mainwindow.hh
  src/preferences.hh
  src/library.hh
  src/tools.hh
  src/download.hh
  src/song-editor.hh
  src/highlighter.hh
  src/dialog-new-song.hh
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
set (TRANSLATIONS
lang/songbook_en.ts
lang/songbook_fr.ts)

set (COMPILED_TRANSLATIONS
lang/songbook_fr.qm
lang/songbook_en.qm)

add_custom_command (OUTPUT
${COMPILED_TRANSLATIONS}
COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
DEPENDS ${TRANSLATIONS}
WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
#uncomment and run make to update translations ts files from sources
#warning, if uncommented, running make clean will remove ts files
#qt4_create_translation(COMPILED_TRANSLATIONS ${SONGBOOK_CLIENT_SOURCES} ${TRANSLATIONS} )
#-------------------------------------------------------------------------------
# generating executable
ADD_DEFINITIONS("-g -Wall")
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
