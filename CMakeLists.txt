cmake_minimum_required(VERSION 3.0)
set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE ON)
SET(CMAKE_BUILD_TYPE Release)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_MODULE_PATH "${SOURCE_DIR}/cmake/")

if(NOT EXISTS ${CMAKE_MODULE_PATH}/patagui-config.cmake)
  message(FATAL_ERROR "Cannot find: ${CMAKE_MODULE_PATH}/patagui-config.cmake")
endif()

include(${CMAKE_MODULE_PATH}/patagui-config.cmake)

# {{{ Sources
set(PATAGUI_SOURCES
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
  src/find-replace-dialog.cc
  src/search-widget.cc
  src/variant-factory.cc
  src/variant-manager.cc
  src/import-dialog.cc
  src/conflict-dialog.cc
  src/diff_match_patch/diff_match_patch.cpp
  src/patacrep.cc
  )

# header (moc)
set(PATAGUI_QT_HEADER
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
  src/find-replace-dialog.hh
  src/search-widget.hh
  src/variant-factory.hh
  src/variant-manager.hh
  src/import-dialog.hh
  src/conflict-dialog.hh
  src/patacrep.hh
  )

# uis
set(PATAGUI_UIS
  )

# resources (rcs)
set(PATAGUI_RCS
  icons/songbook/songbook.qrc
  icons/macos/macos.qrc
  icons/tango/tango.qrc
#  patagui.rc FIXME
  src/songbook_scripts.qrc
  )

# }}}

# Finding Qt5
find_package(Qt5 CONFIG REQUIRED Concurrent Network Widgets)
list(APPEND QT_LIBRARIES ${Qt5Concurrent_LIBRARIES};${Qt5Network_LIBRARIES};${Qt5Widgets_LIBRARIES})
## Qt Libraries have to appear first, otherwise OS X deployment is confused.
list(APPEND LIBRARIES ${QT_LIBRARIES})

# Small Hack for python3 on Mac OSX, otherwise, python3 is not found, only the system library which is 2.7 so far.
if(APPLE)
    execute_process(COMMAND python3-config --prefix
                    OUTPUT_VARIABLE PYTHON3_PREFIX
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND python3 -c "import sys; print(\".\".join(map(str, sys.version_info[:2])))"
                    OUTPUT_VARIABLE PYTHON3_VERSION
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_STRIP_TRAILING_WHITESPACE)
    SET(PYTHON_INCLUDE_DIR  ${PYTHON3_PREFIX}/include/python${PYTHON3_VERSION}m)
    SET(PYTHON_LIBRARY ${PYTHON3_PREFIX}/Python)
endif(APPLE)

# Finding Python 3
find_package(PythonLibs 3.3 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
list(APPEND LIBRARIES ${PYTHON_LIBRARIES})

# Setup PythonQt
include(${CMAKE_MODULE_PATH}/pythonqt-config.cmake)
include_directories(${PYTHONQT_INCLUDE_DIR})
list(APPEND LIBRARIES ${PYTHONQT_LIBRARIES})

# Setup Yaml-cpp
include(${CMAKE_MODULE_PATH}/yaml-cpp-config.cmake)
include_directories(${YAML-CPP_INCLUDE_DIR})
list(APPEND LIBRARIES ${YAML-CPP_LIBRARIES})

# Setup Boost (used in Yaml-CPP)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
list(APPEND LIBRARIES ${Boost_LIBRARIES})

# Use LibArchive
if(ENABLE_LIBRARY_DOWNLOAD)
    set(QT_USE_QTNETWORK true)
    LIST(APPEND QT_LIBRARIES ${Qt5Network_LIBRARIES})
    include_directories(${LibArchive_INCLUDE_DIRS})
    LIST(APPEND LIBRARIES ${LibArchive_LIBRARIES})
endif(ENABLE_LIBRARY_DOWNLOAD)

# Use Hunspell
if(ENABLE_SPELLCHECK)
    include_directories(${HUNSPELL_INCLUDE_DIR})
    LIST(APPEND LIBRARIES ${HUNSPELL_LIBRARIES})
endif(ENABLE_SPELLCHECK)

# Qt Property Editor configuration
include(${CMAKE_MODULE_PATH}/qt-property-editor-config.cmake)

# Local Includes
include_directories(
    ${SOURCE_DIR}
    ${SOURCE_DIR}/src/
    ${BUILD_DIR}
)

# process Qt special files
qt5_wrap_cpp(PATAGUI_MOCS ${PATAGUI_QT_HEADER})
qt5_add_resources(PATAGUI_RESSOURCES ${PATAGUI_RCS})
qt5_wrap_ui(PATAGUI_UI_HDRS ${PATAGUI_UIS})

# {{{ Apple specific stuff
if(APPLE)
  include(cmake/CMakeLists-Mac.cmake)
endif(APPLE)
# }}}

# generating executable
if(NOT APPLE)
    add_executable(${PATAGUI_APPLICATION_NAME} WIN32
        ${PATAGUI_SOURCES}
        ${PATAGUI_MOCS}
        ${PATAGUI_RESSOURCES}
        ${COMPILED_TRANSLATIONS}
        ${QT_PROPERTY_EDITOR_SOURCES}
        ${PATAGUI_UI_HDRS}
    )
endif(NOT APPLE)

if(WIN32)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-enable-auto-import")
endif(WIN32)

# Setup Libraries for linking
target_link_libraries(${PATAGUI_APPLICATION_NAME} ${LIBRARIES})
add_dependencies(${PATAGUI_APPLICATION_NAME} PythonQt-External)
add_dependencies(${PATAGUI_APPLICATION_NAME} Yaml-cpp-External)
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
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
#uncomment and run `make translations_target` to update translations ts files from sources
#warning, if uncommented, running make clean will remove ts files
#qt4_create_translation(COMPILED_TRANSLATIONS ${PATAGUI_SOURCES} ${TRANSLATIONS} )
# }}}

# {{{ Manpages
if(GENERATE_MANPAGES)

  set(PATAGUI_MAN_SRCS
    ${SOURCE_DIR}/manpages/Patagui.1.txt
    )

  set(PATAGUI_MAN_LANGS default fr)

  foreach(lang ${PATAGUI_MAN_LANGS})

    foreach(txtfile ${PATAGUI_MAN_SRCS})
      # figure the base name of the file (ie "patagui.1")
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
  install(TARGETS ${PATAGUI_APPLICATION_NAME} RUNTIME DESTINATION bin)
  if(GENERATE_MANPAGES)
    if(COMPRESS_MANPAGES)
      set(regex "\\.(xml|txt|[0-9])$")
    else()
      set(regex "\\.(xml|txt|gz)$")
    endif()
    install(DIRECTORY ${BUILD_DIR}/${relpath}/ DESTINATION ${PATAGUI_MAN_PATH}
      REGEX ${regex} EXCLUDE)
  endif()
  install(FILES ${PATAGUI_APPLICATION_NAME}.desktop DESTINATION share/applications)
  install(FILES icons/songbook/256x256/Patagui.png DESTINATION share/icons)
  install(FILES ${COMPILED_TRANSLATIONS} DESTINATION ${PATAGUI_DATA_PATH}/lang)
endif(NOT APPLE)
# }}}
