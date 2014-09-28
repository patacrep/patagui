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
  qt5_generate_moc(${file} ${moc_file})
  set(qtpropertyeditor_MOC ${qtpropertyeditor_MOC} ${moc_file})
endforeach(file)

# Process MOC on QtPropertyEditor srcs
foreach(file ${qtpropertyeditor_SRCS_NEED_MOCS})
  string(REGEX REPLACE "${qpe_DIR}/(.*)\\.cpp" "${qpe_BUILD_DIR}/\\1.moc" moc_file "${file}" )
  qt5_generate_moc(${file} ${moc_file})
  set(qtpropertyeditor_MOC ${qtpropertyeditor_MOC} ${moc_file})
endforeach(file)

set_source_files_properties(${qtpropertyeditor_MOC}
  PROPERTIES
  HEADER_FILE_ONLY true
)

# qtpropertybrowserutils_p.h is a special file, do not treat him as the other
# header files.
qt5_generate_moc(
  ${qpe_DIR}/qtpropertybrowserutils_p.h
  ${qpe_BUILD_DIR}/moc_qtpropertybrowserutils_p.cpp
)

set(qtpropertyeditor_MOC
  ${qtpropertyeditor_MOC} 
  ${qpe_BUILD_DIR}/moc_qtpropertybrowserutils_p.cpp
)

qt5_add_resources(qtpropertyeditor_RESOURCES ${qpe_DIR}/qtpropertybrowser.qrc)

set(QT_PROPERTY_EDITOR_SOURCES
  ${qtpropertyeditor_SRCS}
    ${qtpropertyeditor_MOC}
    ${qtpropertyeditor_RESOURCES}
    )
