cmake_minimum_required(VERSION 2.6)
set(CMAKE_COLOR_MAKEFILE ON)
#set(CMAKE_VERBOSE_MAKEFILE OFF)

set(SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(BUILD_DIR ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_MODULE_PATH "${SOURCE_DIR}/cmake/")

if(NOT EXISTS ${SOURCE_DIR}/songbook-client-config.cmake)
  message(FATAL_ERROR "songbook-client-config.cmake")
endif()

include(songbook-client-config.cmake)

find_package(LibArchive REQUIRED)

find_package(Qt4 REQUIRED)
set(QT_USE_QTNETWORK true)
set(QT_USE_QTSVG true)
set(QT_USE_QTSCRIPT true)

include(${QT_USE_FILE})

include(qt-property-editor-config.cmake)

include_directories(
  ${SOURCE_DIR}
  ${SOURCE_DIR}/src/
  ${BUILD_DIR}
  ${LibArchive_INCLUDE_DIRS}
  )

# {{{ Sources
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
  src/song-sort-filter-proxy-model.cc
  src/filter-lineedit.cc
  src/utils/lineedit.cc
  src/utils/lineedit.cc
  src/utils/utils.cc
  src/build-engine/make-songbook.cc
  src/code-editor.cc
  src/tab-widget.cc
  src/library-download.cc
  src/song-panel.cc
  src/notification.cc
  src/identity-proxy-model.cc
  src/song-item-delegate.cc
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
  src/song-sort-filter-proxy-model.hh
  src/filter-lineedit.hh
  src/utils/lineedit.hh
  src/utils/lineedit_p.hh
  src/build-engine/make-songbook.hh
  src/code-editor.hh
  src/tab-widget.hh
  src/library-download.hh
  src/song-panel.hh
  src/notification.hh
  src/identity-proxy-model.hh
  src/song-item-delegate.hh
  )
# add header for binding between C/C++ and objective-C
# to be able to call Sparkle (Autoupdate) on Mac os
# ressources
#should be moved in mac only and use 'LIST' to append
set(SONGBOOK_CLIENT_RCS
  songbook.qrc
  icons/macos/macos.qrc
  icons/tango/tango.qrc
  songbook.rc
  )

set(SONGBOOK_CLIENT_MAN_SRCS
  ${SOURCE_DIR}/manpages/songbook-client.1.txt
  )
set(SONGBOOK_CLIENT_MAN_LANGS fr)
# }}}

# process Qt special files
qt4_wrap_cpp(SONGBOOK_CLIENT_MOCS ${SONGBOOK_CLIENT_QT_HEADER})
qt4_add_resources(SONGBOOK_CLIENT_RESSOURCES ${SONGBOOK_CLIENT_RCS})

# generating executable
add_definitions("-g -Wall")
if(APPLE)
  include(${CMAKE_MODULE_PATH}/CMakeLists-Mac.cmake)
else(APPLE)
  add_executable(${SONGBOOK_CLIENT_APPLICATION_NAME}
    ${SONGBOOK_CLIENT_SOURCES}
    ${SONGBOOK_CLIENT_MOCS}
    ${SONGBOOK_CLIENT_RESSOURCES}
    ${COMPILED_TRANSLATIONS}
    ${QT_PROPERTY_EDITOR_SOURCES}
    )
endif(APPLE)

if(WIN32)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-enable-auto-import")
endif(WIN32)

target_link_libraries(${SONGBOOK_CLIENT_APPLICATION_NAME} ${QT_LIBRARIES} ${LibArchive_LIBRARIES})

# {{{ Internationalization
set (TRANSLATIONS
  lang/songbook_en.ts
  lang/songbook_fr.ts)

set (COMPILED_TRANSLATIONS
  lang/songbook_fr.qm
  lang/songbook_en.qm)

add_custom_command(
  COMMAND "${QT_LRELEASE_EXECUTABLE}" ${TRANSLATIONS}
  OUTPUT ${COMPILED_TRANSLATIONS}
  DEPENDS ${TRANSLATIONS}
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
#uncomment and run make to update translations ts files from sources
#warning, if uncommented, running make clean will remove ts files
#qt4_create_translation(COMPILED_TRANSLATIONS ${SONGBOOK_CLIENT_SOURCES} ${TRANSLATIONS} )
# }}}

# {{{ Manpages
if(GENERATE_MANPAGES)

  # add the default translation to the list of languages
  set(SONGBOOK_CLIENT_MAN_LANGS default ${SONGBOOK_CLIENT_MAN_LANGS})

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
  install(FILES icons/songbook-client.png DESTINATION share/icons)
  install(FILES ${COMPILED_TRANSLATIONS} DESTINATION ${SONGBOOK_CLIENT_DATA_PATH}/lang)
endif(NOT APPLE)
# }}}
