cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_COLOR_MAKEFILE ON)
#set(CMAKE_VERBOSE_MAKEFILE OFF)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_MODULE_PATH "${SOURCE_DIR}/cmake/")

if(NOT EXISTS ${CMAKE_MODULE_PATH}/songbook-client-config.cmake)
  message(FATAL_ERROR "Cannot find: ${CMAKE_MODULE_PATH}/songbook-client-config.cmake")
endif()

include(${CMAKE_MODULE_PATH}/songbook-client-config.cmake)

# {{{ Sources
set(SONGBOOK_CLIENT_SOURCES
  src/main.cc
  src/main-window.cc
  src/preferences.cc
  src/library.cc
  src/song.cc
  src/library-view.cc
  src/songbook.cc
  src/song-editor.cc
  src/song-header-editor.cc
  src/song-code-editor.cc
  src/song-highlighter.cc
  src/logs-highlighter.cc
  src/label.cc
  src/chord.cc
  src/diagram-area.cc
  src/diagram-editor.cc
  src/chord-list-model.cc
  src/progress-bar.cc
  src/file-chooser.cc
  src/song-sort-filter-proxy-model.cc
  src/filter-lineedit.cc
  src/utils/lineedit.cc
  src/code-editor.cc
  src/tab-widget.cc
  src/notification.cc
  src/identity-proxy-model.cc
  src/song-item-delegate.cc
  src/make-songbook-process.cc
  src/find-replace-dialog.cc
  src/search-widget.cc
  src/variant-factory.cc
  src/variant-manager.cc
  src/import-dialog.cc
  src/conflict-dialog.cc
  src/diff_match_patch/diff_match_patch.cpp
  )

# header (moc)
set(SONGBOOK_CLIENT_QT_HEADER
  src/main-window.hh
  src/preferences.hh
  src/library.hh
  src/library-view.hh
  src/songbook.hh
  src/song-editor.hh
  src/song-header-editor.hh
  src/song-code-editor.hh
  src/song-highlighter.hh
  src/logs-highlighter.hh
  src/label.hh
  src/chord.hh
  src/diagram-area.hh
  src/diagram-editor.hh
  src/chord-list-model.hh
  src/progress-bar.hh
  src/file-chooser.hh
  src/song-sort-filter-proxy-model.hh
  src/filter-lineedit.hh
  src/utils/lineedit.hh
  src/utils/lineedit_p.hh
  src/code-editor.hh
  src/tab-widget.hh
  src/notification.hh
  src/identity-proxy-model.hh
  src/song-item-delegate.hh
  src/make-songbook-process.hh
  src/find-replace-dialog.hh
  src/search-widget.hh
  src/variant-factory.hh
  src/variant-manager.hh
  src/import-dialog.hh
  src/conflict-dialog.hh
  )

# uis
set(SONGBOOK_CLIENT_UIS
  )

# resources (rcs)
set(SONGBOOK_CLIENT_RCS
  icons/songbook/songbook.qrc
  icons/macos/macos.qrc
  icons/tango/tango.qrc
  songbook.rc
  )

# }}}

# Finding Qt5
find_package(Qt5 CONFIG REQUIRED Network Widgets Script)
list(APPEND QT_LIBRARIES ${Qt5Widgets_LIBRARIES};${Qt5Script_LIBRARIES})

################ DIRTY HACK FOR IT TO WORK FOR NOW ###################
#### Problem lies with cmake, can't properly find python3 on Mac #####
SET(PYTHON_INCLUDE_DIR  /usr/local/opt/python3/Frameworks/Python.framework/Versions/3.4/include/python3.4m)
SET(PYTHON_LIBRARY /usr/local/opt/python3/Frameworks/Python.framework/Versions/3.4/lib)
############################# End of Hack ############################

# Finding Python 3
find_package(PythonLibs 3.3 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
list(APPEND LIBRARIES ${PYTHON_LIBRARIES})

# Setup PythonQt
include(${CMAKE_MODULE_PATH}/FindPythonQt.cmake)
list(APPEND LIBRARIES ${PYTHONQT_LIBRARIES})
include_directories(${PYTHONQT_INCLUDE_DIR})

# Use LibArchive
if(ENABLE_LIBRARY_DOWNLOAD)
  set(QT_USE_QTNETWORK true)
  LIST(APPEND QT_LIBRARIES ${Qt5Network_LIBRARIES})
  include_directories(${LibArchive_INCLUDE_DIRS})
  LIST(APPEND LIBRARIES ${LibArchive_LIBRARIES})
endif()

# Use Hunspell
if(ENABLE_SPELLCHECK)
  include_directories(${HUNSPELL_INCLUDE_DIR})
  LIST(APPEND LIBRARIES ${HUNSPELL_LIBRARIES})
endif()

# Qt Property Editor configuration
include(${CMAKE_MODULE_PATH}/qt-property-editor-config.cmake)

# Local Includes
include_directories(
  ${SOURCE_DIR}
  ${SOURCE_DIR}/src/
  ${BUILD_DIR}
  )

# process Qt special files
qt5_wrap_cpp(SONGBOOK_CLIENT_MOCS ${SONGBOOK_CLIENT_QT_HEADER})
qt5_add_resources(SONGBOOK_CLIENT_RESSOURCES ${SONGBOOK_CLIENT_RCS})
qt5_wrap_ui(SONGBOOK_CLIENT_UI_HDRS ${SONGBOOK_CLIENT_UIS})

# {{{ Apple specific stuff
if(APPLE)
  include(cmake/CMakeLists-Mac.cmake)
endif(APPLE)
# }}}

# generating executable
if(NOT APPLE)
  add_executable(${SONGBOOK_CLIENT_APPLICATION_NAME} WIN32
    ${SONGBOOK_CLIENT_SOURCES}
    ${SONGBOOK_CLIENT_MOCS}
    ${SONGBOOK_CLIENT_RESSOURCES}
    ${COMPILED_TRANSLATIONS}
    ${QT_PROPERTY_EDITOR_SOURCES}
    ${SONGBOOK_CLIENT_UI_HDRS}
    )
endif(NOT APPLE)

if(WIN32)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-enable-auto-import")
endif(WIN32)

# Setup Libraries for linking
list(APPEND LIBRARIES ${QT_LIBRARIES})
target_link_libraries(${SONGBOOK_CLIENT_APPLICATION_NAME} ${LIBRARIES})

# {{{ Internationalization
set (TRANSLATIONS
  lang/songbook_en.ts
  lang/songbook_it.ts
  lang/songbook_fr.ts
  lang/songbook_cs.ts)

set (COMPILED_TRANSLATIONS
  lang/songbook_fr.qm
  lang/songbook_it.qm
  lang/songbook_en.qm
  lang/songbook_cs.qm)

add_custom_target(
  translations_target
  COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
  DEPENDS ${TRANSLATIONS}
  COMMENT "Updating translations"
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
#uncomment and run `make translations_target` to update translations ts files from sources
#warning, if uncommented, running make clean will remove ts files
#qt4_create_translation(COMPILED_TRANSLATIONS ${SONGBOOK_CLIENT_SOURCES} ${TRANSLATIONS} )
# }}}

# {{{ Manpages
if(GENERATE_MANPAGES)

  set(SONGBOOK_CLIENT_MAN_SRCS
    ${SOURCE_DIR}/manpages/songbook-client.1.txt
    )

  set(SONGBOOK_CLIENT_MAN_LANGS default fr)

  foreach(lang ${SONGBOOK_CLIENT_MAN_LANGS})

    foreach(txtfile ${SONGBOOK_CLIENT_MAN_SRCS})
      # figure the base name of the file (ie "songbook-client.1")
      GET_FILENAME_COMPONENT(tmpname ${txtfile} NAME)
      string(REGEX REPLACE ".txt\$" "" basename ${tmpname})

      # figure the relative path of the file
      GET_FILENAME_COMPONENT(tmppath ${txtfile} PATH)
      string(REPLACE ${SOURCE_DIR}/ "" relpath ${tmppath})

      # figure the manpage section to install to from filename
      string(REGEX REPLACE "^.*\\.([0-9])$" "\\1" section ${basename})

      # construct the language specific versions of the basename and path
      if (lang STREQUAL default)
        set(basename2 ${basename})
        set(relpath2 ${relpath}/man${section})
      else()
        set(basename2 ${basename}.${lang})
        set(relpath2 ${relpath}/${lang}/man${section})
      endif()

      # create the build directory (if it does not exist)
      file(MAKE_DIRECTORY ${BUILD_DIR}/${relpath2})

      # set the final filenames
      set(txtfile ${SOURCE_DIR}/${relpath}/${basename2}.txt)
      set(xmlfile ${BUILD_DIR}/${relpath2}/${basename}.xml)
      set(gzfile  ${BUILD_DIR}/${relpath2}/${basename}.gz)
      set(manfile ${BUILD_DIR}/${relpath2}/${basename})

      add_custom_command(
        COMMAND ${ASCIIDOC_EXECUTABLE} -d manpage -b docbook -o ${xmlfile} - < ${txtfile}
        WORKING_DIRECTORY ${BUILD_DIR}/${relpath2}
        OUTPUT  ${xmlfile}
        DEPENDS ${txtfile}
        VERBATIM)
      add_custom_command(
        COMMAND ${XMLTO_EXECUTABLE} man ${xmlfile}
        OUTPUT  ${manfile}
        WORKING_DIRECTORY ${BUILD_DIR}/${relpath2}
        DEPENDS ${xmlfile})

      if(COMPRESS_MANPAGES)
        add_custom_command(
          COMMAND ${GZIP_EXECUTABLE} < ${manfile} > ${gzfile}
          OUTPUT  ${gzfile}
          WORKING_DIRECTORY ${BUILD_DIR}/${relpath2}
          DEPENDS ${manfile}
          VERBATIM)

        set(MAN_FILES ${MAN_FILES} ${gzfile})
      else()
        set(MAN_FILES ${MAN_FILES} ${manfile})
      endif()
    endforeach()

  endforeach()

  add_custom_target(man ALL DEPENDS ${MAN_FILES})
endif()
# }}}

# {{{ Installation
if(NOT APPLE)
  install(TARGETS ${SONGBOOK_CLIENT_APPLICATION_NAME} RUNTIME DESTINATION bin)
  if(GENERATE_MANPAGES)
    if(COMPRESS_MANPAGES)
      set(regex "\\.(xml|txt|[0-9])$")
    else()
      set(regex "\\.(xml|txt|gz)$")
    endif()
    install(DIRECTORY ${BUILD_DIR}/${relpath}/ DESTINATION ${SONGBOOK_CLIENT_MAN_PATH}
      REGEX ${regex} EXCLUDE)
  endif()
  install(FILES ${SONGBOOK_CLIENT_APPLICATION_NAME}.desktop DESTINATION share/applications)
  install(FILES icons/songbook/256x256/songbook-client.png DESTINATION share/icons)
  install(FILES ${COMPILED_TRANSLATIONS} DESTINATION ${SONGBOOK_CLIENT_DATA_PATH}/lang)
endif(NOT APPLE)
# }}}
