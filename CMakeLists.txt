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
set(QT_USE_QTSCRIPT true)
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
  src/tools.cc
  src/download.cc
  src/song-editor.cc
  src/highlighter.cc
  src/dialog-new-song.cc
  src/label.cc
  src/unit-property-manager.cc
  src/unit-factory.cc
  src/file-property-manager.cc
  src/file-factory.cc
  )
#-------------------------------------------------------------------------------
# header (moc)
set(SONGBOOK_CLIENT_QT_HEADER
  src/mainwindow.hh
  src/preferences.hh
  src/library.hh
  src/tools.hh
  src/download.hh
  src/songbook.hh
  src/song-editor.hh
  src/highlighter.hh
  src/dialog-new-song.hh
  src/label.hh
  src/unit-property-manager.hh
  src/unit-factory.hh
  src/file-property-manager.hh
  src/file-factory.hh
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
# Support for qtpropertybrowser
set(qpe_DIR src/qtpropertybrowser)
set(qpe_BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR}/src)

include_directories(${qpe_DIR} ${qpe_BUILD_DIR})

set(
  qtpropertyeditor_HEADER_NEED_MOCS
  ${qpe_DIR}/qtpropertybrowser.h
  ${qpe_DIR}/qtpropertymanager.h
  ${qpe_DIR}/qteditorfactory.h
  ${qpe_DIR}/qtvariantproperty.h
  ${qpe_DIR}/qttreepropertybrowser.h
  ${qpe_DIR}/qtbuttonpropertybrowser.h
  ${qpe_DIR}/qtgroupboxpropertybrowser.h
  )

set(
  qtpropertyeditor_SRCS_NEED_MOCS
  ${qpe_DIR}/qtpropertymanager.cpp
  ${qpe_DIR}/qteditorfactory.cpp
  ${qpe_DIR}/qttreepropertybrowser.cpp
  )

set(
  qtpropertyeditor_SRCS
  ${qtpropertyeditor_SRCS_NEED_MOCS}
  ${qpe_DIR}/qtpropertybrowser.cpp
  ${qpe_DIR}/qtvariantproperty.cpp
  ${qpe_DIR}/qtbuttonpropertybrowser.cpp
  ${qpe_DIR}/qtgroupboxpropertybrowser.cpp
  ${qpe_DIR}/qtpropertybrowserutils.cpp
  )

# Process MOC on QtPropertyEditor headers
FOREACH(file ${qtpropertyeditor_HEADER_NEED_MOCS})
  STRING(REGEX REPLACE "${qpe_DIR}/(.*)\\.h" "${qpe_BUILD_DIR}/moc_\\1.cpp" moc_file "${file}" )
  QT4_GENERATE_MOC(${file} ${moc_file})
  SET(qtpropertyeditor_MOC ${qtpropertyeditor_MOC} ${moc_file})
ENDFOREACH(file)

# Process MOC on QtPropertyEditor srcs
FOREACH(file ${qtpropertyeditor_SRCS_NEED_MOCS})
  STRING(REGEX REPLACE "${qpe_DIR}/(.*)\\.cpp" "${qpe_BUILD_DIR}/\\1.moc" moc_file "${file}" )
  QT4_GENERATE_MOC(${file} ${moc_file})
  SET(qtpropertyeditor_MOC ${qtpropertyeditor_MOC} ${moc_file})
ENDFOREACH(file)

set_source_files_properties(${qtpropertyeditor_MOC}
  PROPERTIES
  HEADER_FILE_ONLY true
)

# qtpropertybrowserutils_p.h is a special file, do not treat him as the other
# header files.
QT4_GENERATE_MOC(
  ${qpe_DIR}/qtpropertybrowserutils_p.h
  ${qpe_BUILD_DIR}/moc_qtpropertybrowserutils_p.cpp
)

SET(qtpropertyeditor_MOC
  ${qtpropertyeditor_MOC} 
  ${qpe_BUILD_DIR}/moc_qtpropertybrowserutils_p.cpp
)

QT4_ADD_RESOURCES(qtpropertyeditor_RESOURCES ${qpe_DIR}/qtpropertybrowser.qrc)
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
  ${qtpropertyeditor_SRCS}
  ${qtpropertyeditor_MOC}
  ${qtpropertyeditor_RESOURCES}
)
target_link_libraries(songbook-client ${QT_LIBRARIES})
#-------------------------------------------------------------------------------
# install instructions
install(TARGETS songbook-client RUNTIME DESTINATION bin)
install(FILES songbook-client.desktop DESTINATION share/applications)
install(FILES icons/patacrep.png DESTINATION share/icons)
install(FILES ${COMPILED_TRANSLATIONS} DESTINATION share/songbook-client/translations)
