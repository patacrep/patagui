cmake_minimum_required(VERSION 2.6)
project(songbook-client)
set( PROGNAME songbook-client )
if( APPLE )
  set( PROGNAME Songbook-Client )
  set( MACOSX_BUNDLE_ICON_FILE livre.icns )
  set( CPACK_BUNDLE_NAME Sonbook Client With Space)
  #versions
  set( MACOSX_BUNDLE_VERSION 0.5.1 )
  set( MACOSX_BUNDLE_SHORT_VERSION_STRING 0.5.1 )
  set( MACOSX_BUNDLE_LONG_VERSION_STRING "Version 0.5.1" )

  #name of the preference file, usefull if we need to keep the same preference file
  #upgrade after upgrade without loosing preferences
  set(MACOSX_BUNDLE_GUI_IDENTIFIER com.patacrep.songbook-client)

  #need to copy the icns (icon) file in the buils dir
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/macos_specific/livre.icns
    ${CMAKE_CURRENT_BINARY_DIR}/livre.icns COPYONLY)

  #need to check a few things for autoupdate
  # this will set the UPPERCASE Variable to the right path 
  # for soft/Hard linknig of the framwork.
  find_library(SPARKLE_FRAMEWORK Sparkle)
  find_library(APPKIT_FRAMEWORK AppKit)

  # we need to copy the public key to check the updates
  set(PUBLIC_KEY "${CMAKE_CURRENT_SOURCE_DIR}/macos_specific/dsa_pub_songbookclient.pem")
  SET_SOURCE_FILES_PROPERTIES(
     ${PUBLIC_KEY}
     PROPERTIES
     MACOSX_PACKAGE_LOCATION Resources
   )
endif( APPLE )

#-------------------------------------------------------------------------------
set( CMAKE_COLOR_MAKEFILE ON )
#set( CMAKE_VERBOSE_MAKEFILE ON )
#set( CMAKE_INCLUDE_CURRENT_DIR TRUE )
#-------------------------------------------------------------------------------
# process options and parameters
if(DEFINED PREFIX)
  set(CMAKE_INSTALL_PREFIX ${PREFIX})
endif()
#-------------------------------------------------------------------------------
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
#-------------------------------------------------------------------------------
find_package(LibArchive REQUIRED)
include_directories(${LibArchive_INCLUDE_DIRS})

find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
set(QT_USE_QTSVG true)
set(QT_USE_QTSCRIPT true)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
message(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# sources
set(SONGBOOK_CLIENT_SOURCES
  src/main.cc
  src/main-window.cc
  src/preferences.cc
  src/library.cc
  src/library-view.cc
  src/songbook.cc
  src/build-engine.cc
  src/song-editor.cc
  src/highlighter.cc
  src/dialog-new-song.cc
  src/label.cc
  src/unit-property-manager.cc
  src/unit-factory.cc
  src/file-property-manager.cc
  src/file-factory.cc
  src/file-chooser.cc
  src/songSortFilterProxyModel.cc
  src/filter-lineedit.cc
  src/utils/lineedit.cc
  src/utils/lineedit.cc
  src/utils/utils.cc
  src/build-engine/make-songbook.cc
  src/code-editor.cc
  src/tab-widget.cc
  src/library-download.cc
  src/song-panel.cc
  src/songbook-panel.cc
  src/notification.cc
  src/songbook-model.cc
  )

# header (moc)
set(SONGBOOK_CLIENT_QT_HEADER
  src/main-window.hh
  src/preferences.hh
  src/library.hh
  src/library-view.hh
  src/build-engine.hh
  src/songbook.hh
  src/song-editor.hh
  src/highlighter.hh
  src/dialog-new-song.hh
  src/label.hh
  src/unit-property-manager.hh
  src/unit-factory.hh
  src/file-property-manager.hh
  src/file-factory.hh
  src/file-chooser.hh
  src/songSortFilterProxyModel.hh
  src/filter-lineedit.hh
  src/utils/lineedit.hh
  src/utils/lineedit_p.hh
  src/build-engine/make-songbook.hh
  src/code-editor.hh
  src/tab-widget.hh
  src/library-download.hh
  src/song-panel.hh
  src/songbook-panel.hh
  src/notification.hh
  src/songbook-model.hh
  )
# add header for binding between C/C++ and objective-C
# to be able to call Sparkle (Autoupdate) on Mac os
if(APPLE)
  LIST(
    APPEND SONGBOOK_CLIENT_QT_HEADER
    macos_specific/sparkle/src/AutoUpdater.h
    macos_specific/sparkle/src/CocoaInitializer.h
    macos_specific/sparkle/src/SparkleAutoUpdater.h
    )
  LIST(
    APPEND SONGBOOK_CLIENT_SOURCES
    macos_specific/sparkle/src/AutoUpdater.cpp
    macos_specific/sparkle/src/CocoaInitializer.mm
    macos_specific/sparkle/src/SparkleAutoUpdater.mm
    )
endif(APPLE)
# ressources
set(SONGBOOK_CLIENT_RCS songbook.qrc icons/macos/macos.qrc icons/tango/tango.qrc songbook.rc)

# process Qt special files
include(${QT_USE_FILE})
qt4_wrap_cpp(SONGBOOK_CLIENT_MOCS ${SONGBOOK_CLIENT_QT_HEADER})
qt4_add_resources(SONGBOOK_CLIENT_RESSOURCES ${SONGBOOK_CLIENT_RCS})
include_directories (${CMAKE_CURRENT_BINARY_DIR} src)

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
foreach(file ${qtpropertyeditor_HEADER_NEED_MOCS})
  string(REGEX REPLACE "${qpe_DIR}/(.*)\\.h" "${qpe_BUILD_DIR}/moc_\\1.cpp" moc_file "${file}" )
  qt4_generate_moc(${file} ${moc_file})
  set(qtpropertyeditor_MOC ${qtpropertyeditor_MOC} ${moc_file})
endforeach(file)

# Process MOC on QtPropertyEditor srcs
foreach(file ${qtpropertyeditor_SRCS_NEED_MOCS})
  string(REGEX REPLACE "${qpe_DIR}/(.*)\\.cpp" "${qpe_BUILD_DIR}/\\1.moc" moc_file "${file}" )
  qt4_generate_moc(${file} ${moc_file})
  set(qtpropertyeditor_MOC ${qtpropertyeditor_MOC} ${moc_file})
endforeach(file)

set_source_files_properties(${qtpropertyeditor_MOC}
  PROPERTIES
  HEADER_FILE_ONLY true
)

# qtpropertybrowserutils_p.h is a special file, do not treat him as the other
# header files.
qt4_generate_moc(
  ${qpe_DIR}/qtpropertybrowserutils_p.h
  ${qpe_BUILD_DIR}/moc_qtpropertybrowserutils_p.cpp
)

set(qtpropertyeditor_MOC
  ${qtpropertyeditor_MOC} 
  ${qpe_BUILD_DIR}/moc_qtpropertybrowserutils_p.cpp
)

qt4_add_resources(qtpropertyeditor_RESOURCES ${qpe_DIR}/qtpropertybrowser.qrc)
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

# generating executable
add_definitions("-g -Wall")
if( APPLE )
    add_executable( ${PROGNAME} MACOSX_BUNDLE ${SONGBOOK_CLIENT_SOURCES} 
      ${SONGBOOK_CLIENT_MOCS} ${SONGBOOK_CLIENT_RESSOURCES} ${COMPILED_TRANSLATIONS}
      ${qtpropertyeditor_SRCS} ${qtpropertyeditor_MOC} ${qtpropertyeditor_RESOURCES}
      ${PUBLIC_KEY}
      )
    add_custom_command( TARGET ${PROGNAME} POST_BUILD
      #COMMAND mkdir ARGS ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
      COMMAND mkdir ARGS ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/lang
      COMMAND cp ARGS ${MACOSX_BUNDLE_ICON_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources
      COMMAND cp ARGS ../lang/*.qm ${CMAKE_CURRENT_BINARY_DIR}/${PROGNAME}.app/Contents/Resources/lang
      )
    # we need to add SPARKLE and APPKIT as '-framework'
    target_link_libraries(${PROGNAME} ${APPKIT_FRAMEWORK})
    target_link_libraries(${PROGNAME} ${SPARKLE_FRAMEWORK})

	#genération d'une plist custom pour sparkle from a plist.in whete @VARAIBLE@ are replaces by the content of VARAIBLE
    set_target_properties( ${PROGNAME}  PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/macos_specific/Info.plist.in )
else( APPLE )
    add_executable(${PROGNAME}
      ${SONGBOOK_CLIENT_SOURCES} 
      ${SONGBOOK_CLIENT_MOCS} 
      ${SONGBOOK_CLIENT_RESSOURCES} 
      ${COMPILED_TRANSLATIONS}
      ${qtpropertyeditor_SRCS}
      ${qtpropertyeditor_MOC}
      ${qtpropertyeditor_RESOURCES}
    )
endif( APPLE )

target_link_libraries(${PROGNAME} ${QT_LIBRARIES})
if(WIN32)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-enable-auto-import")
endif(WIN32)
target_link_libraries(${PROGNAME} ${LibArchive_LIBRARIES})

# install instructions
if(NOT APPLE)
  install(TARGETS ${PROGNAME} RUNTIME DESTINATION bin)
  install(FILES ${PROGNAME}.desktop DESTINATION share/applications)
  install(FILES icons/songbook-client.png DESTINATION share/icons)
  install(FILES ${COMPILED_TRANSLATIONS} DESTINATION share/${PROGNAME}/translations)
endif(NOT APPLE)
