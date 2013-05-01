set(PROJECT_NAME songbook-client)

# If ${SOURCE_DIR} is a git repository VERSION is set to
# `git describe --tags` later.
set(VERSION devel)

set(CODENAME "")

#project(${PROJECT_NAME} C)

option(GENERATE_MANPAGES "generate manpages" ON)
option(COMPRESS_MANPAGES "compress manpages" ON)
option(ENABLE_LIBRARY_DOWNLOAD "allow the application to download songbooks" ON)
option(ENABLE_SPELLCHECK "allow the application to apply spellchecking within song-editor" ON)

# {{{ CFLAGS
if (CMAKE_BUILD_TYPE MATCHES "Release")
  message(STATUS "Compiling in Release mode")
  add_definitions(-O2 -march=native)
elseif( CMAKE_COMPILER_IS_GNUCXX )
  # Add additional GCC options.
  add_definitions(
    -ggdb3 -fno-strict-aliasing -Wall -Wextra
    -Wchar-subscripts -Wundef -Wcast-align -Wwrite-strings
    -Wsign-compare -Wunused -Wuninitialized -Winit-self
    -Wpointer-arith -Wredundant-decls -Wformat-nonliteral
    -Wmissing-format-attribute -Wformat-security
    )
elseif( CMAKE_CXX_COMPILER MATCHES "clang" )
  add_definitions( -Wall -Wextra )
endif()

if(NOT WIN32)
  add_definitions( -fvisibility=hidden )
endif()
# }}}

# {{{ Find external utilities
macro(a_find_program var prg req)
  set(required ${req})
  find_program(${var} ${prg})
  if(NOT ${var})
    message(STATUS "${prg} not found.")
    if(required)
      message(FATAL_ERROR "${prg} is required to build ${PROJECT_NAME}")
    endif()
  else()
    message(STATUS "${prg} -> ${${var}}")
  endif()
endmacro()

a_find_program(GIT_EXECUTABLE git FALSE)
a_find_program(HOSTNAME_EXECUTABLE hostname FALSE)
# programs needed for man pages
a_find_program(ASCIIDOC_EXECUTABLE asciidoc FALSE)
a_find_program(XMLTO_EXECUTABLE xmlto FALSE)
a_find_program(GZIP_EXECUTABLE gzip FALSE)
# }}}

if(ENABLE_LIBRARY_DOWNLOAD)
  find_package(LibArchive)
  if(LibArchive_FOUND)
    message(STATUS "Library download enabled")
  else()
    set(ENABLE_LIBRARY_DOWNLOAD false)
    message(STATUS "Library download disabled (can't find libarchive)")
  endif()
else()
  message(STATUS "Library download disabled")
endif()

if(ENABLE_SPELLCHECK)
  find_package(HUNSPELL)
  if(HUNSPELL_FOUND)
    message(STATUS "Spellchecking enabled")
  else()
    set(ENABLE_SPELLCHECK false)
    message(STATUS "Spellchecking disabled (can't find hunspell)")
  endif()
else()
  message(STATUS "Spellchecking disabled")
endif()

# {{{ Check if documentation can be build
if(GENERATE_MANPAGES)
  if(NOT ASCIIDOC_EXECUTABLE OR NOT XMLTO_EXECUTABLE
      OR (COMPRESS_MANPAGES AND NOT GZIP_EXECUTABLE)
      OR NOT EXISTS ${SOURCE_DIR}/manpages/)
    if(NOT ASCIIDOC_EXECUTABLE)
      SET(missing "asciidoc")
    endif()
    if(NOT XMLTO_EXECUTABLE)
      SET(missing ${missing} " xmlto")
    endif()
    if(COMPRESS_MANPAGES AND NOT GZIP_EXECUTABLE)
      SET(missing ${missing} " gzip")
    endif()

    message(STATUS "Not generating manpages. Missing: " ${missing})
    set(GENERATE_MANPAGES OFF)
  endif()
endif()
# }}}

# {{{ Version stamp
if(EXISTS ${SOURCE_DIR}/.git/HEAD AND GIT_EXECUTABLE)
  # get current version
  execute_process(
    COMMAND ${GIT_EXECUTABLE} describe --tags
    WORKING_DIRECTORY ${SOURCE_DIR}
    OUTPUT_VARIABLE VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(BUILD_FROM_GIT TRUE)
endif()
# }}}

# {{{ Get hostname
execute_process(
  COMMAND ${HOSTNAME_EXECUTABLE}
  WORKING_DIRECTORY ${SOURCE_DIR}
  OUTPUT_VARIABLE BUILDHOSTNAME
  OUTPUT_STRIP_TRAILING_WHITESPACE)
# }}}

# {{{ Install path and configuration variables
if(DEFINED PREFIX)
  set(PREFIX ${PREFIX} CACHE PATH "install prefix")
  set(CMAKE_INSTALL_PREFIX ${PREFIX})
else()
  set(PREFIX ${CMAKE_INSTALL_PREFIX} CACHE PATH "install prefix")
endif()

# set man path
if(DEFINED SONGBOOK_CLIENT_MAN_PATH)
  set(SONGBOOK_CLIENT_MAN_PATH ${SONGBOOK_CLIENT_MAN_PATH} CACHE PATH "songbook-client manpage directory")
else()
  set(SONGBOOK_CLIENT_MAN_PATH ${PREFIX}/share/man CACHE PATH "songbook-client manpage directory")
endif()

# Hide to avoid confusion
mark_as_advanced(CMAKE_INSTALL_PREFIX)

set(SONGBOOK_CLIENT_APPLICATION_NAME ${PROJECT_NAME})
set(SONGBOOK_CLIENT_VERSION          ${VERSION})
set(SONGBOOK_CLIENT_COMPILE_MACHINE  ${CMAKE_SYSTEM_PROCESSOR})
set(SONGBOOK_CLIENT_COMPILE_HOSTNAME ${BUILDHOSTNAME})
set(SONGBOOK_CLIENT_COMPILE_BY       $ENV{USER})
set(SONGBOOK_CLIENT_RELEASE          ${CODENAME})
set(SONGBOOK_CLIENT_DATA_PATH        ${PREFIX}/share/${SONGBOOK_CLIENT_APPLICATION_NAME})
# }}}

# {{{ Configure files
set(SONGBOOK_CLIENT_CONFIGURE_FILES 
  config.hh.in
  )

macro(a_configure_file file)
  string(REGEX REPLACE ".in\$" "" outfile ${file})
  message(STATUS "Configuring ${outfile}")
  configure_file(${SOURCE_DIR}/${file}
    ${BUILD_DIR}/${outfile}
    ESCAPE_QUOTE
    @ONLY)
endmacro()

foreach(file ${SONGBOOK_CLIENT_CONFIGURE_FILES})
  a_configure_file(${file})
endforeach()
#}}}
