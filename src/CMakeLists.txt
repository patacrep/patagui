#-------------------------------------------------------------------------------
cmake_minimum_required(VERSION 2.6)
#-------------------------------------------------------------------------------
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
message(${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
#-------------------------------------------------------------------------------
find_package(Qt4 REQUIRED)
set(QT_USE_QTSQL true)
#-------------------------------------------------------------------------------
include(${QT_USE_FILE})
#-------------------------------------------------------------------------------
# sources
set(SONGBOOK_CLIENT_SOURCES
  main.cc
  mainwindow.cc
  preferences.cc
  library.cc
  songbook.cc
  header.cc
  tools.cc
  )
#-------------------------------------------------------------------------------
# header (moc)
set(SONGBOOK_CLIENT_QT_HEADER
  mainwindow.hh
  preferences.hh
  library.hh
  tools.hh
  )
#-------------------------------------------------------------------------------
# ressources
set(SONGBOOK_CLIENT_RCS
  application.qrc
  )
#-------------------------------------------------------------------------------
# process Qt special files
qt4_wrap_cpp(SONGBOOK_CLIENT_MOCS ${SONGBOOK_CLIENT_QT_HEADER})
qt4_add_resources(SONGBOOK_CLIENT_RESSOURCES ${SONGBOOK_CLIENT_RCS})
#-------------------------------------------------------------------------------
# internationalization
#set(LANGUAGES fr en)
#
#set(LANGUAGE_TS_FILES)
#foreach(LANGUAGE ${LANGUAGES})
#  set(TS_FILE "${CMAKE_CURRENT_SOURCE_DIR}/langs/songbook_${LANGUAGE}.ts")
#  set(LANGUAGE_TS_FILES ${LANGUAGE_TS_FILES} ${TS_FILE})
#  set_source_files_properties(${TS_FILE} PROPERTIES OUTPUT_LOCATION
#    "${CMAKE_CURRENT_SOURCE_DIR}/../translations")
#endforeach(LANGUAGE ${LANGUAGES})
#if(LANGUAGE_TS_FILES)
#  qt4_create_translation(qm_files ${IHM_SRCS} ${LANGUAGE_TS_FILES})
#endif(LANGUAGE_TS_FILES)
#-------------------------------------------------------------------------------
# generating executable
add_executable(songbook-client
  ${SONGBOOK_CLIENT_SOURCES} 
  ${SONGBOOK_CLIENT_MOCS} 
  ${SONGBOOK_CLIENT_RESSOURCES} 
  ${SONGBOOK_CLIENT_TRANSLATIONS}
)
target_link_libraries(songbook-client ${QT_LIBRARIES})
#-------------------------------------------------------------------------------
# install instructions
install(TARGETS songbook-client RUNTIME DESTINATION bin)
#-------------------------------------------------------------------------------
