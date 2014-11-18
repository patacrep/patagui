	set( SONGBOOK_CLIENT_APPLICATION_NAME Songbook-Client )
	set( MACOSX_BUNDLE_ICON_FILE songbook-client.icns )
	set( CPACK_BUNDLE_NAME Sonbook Client With Space)
	#versions
	set( MACOSX_BUNDLE_VERSION 0.5.1 )
	set( MACOSX_BUNDLE_SHORT_VERSION_STRING 0.5.1 )
	set( MACOSX_BUNDLE_LONG_VERSION_STRING "Version 0.5.1" )

	#name of the preference file, usefull if we need to keep the same preference file
	#upgrade after upgrade without loosing preferences
	set(MACOSX_BUNDLE_GUI_IDENTIFIER com.patacrep.songbook-client)

	# need to copy the icns (icon) file in the buils dir
	# also copy the icons for the .sg and .sb files
	SET_SOURCE_FILES_PROPERTIES(
		${CMAKE_CURRENT_SOURCE_DIR}/macos_specific/book.icns
		${CMAKE_CURRENT_SOURCE_DIR}/macos_specific/song.icns
		PROPERTIES
		MACOSX_PACKAGE_LOCATION Resources
	)
	#set( CMAKE_OSX_ARCHITECTURES "ppc;i386;ppc64;x86_64" 
	#set( CMAKE_OSX_ARCHITECTURES "ppc;i386" 
	#				CACHE STRING "Build architectures for OSX" FORCE)
	
	configure_file(${CMAKE_CURRENT_SOURCE_DIR}/macos_specific/songbook-client.icns
		${CMAKE_CURRENT_BINARY_DIR}/songbook-client.icns COPYONLY)

	IF(EXISTS "/Library/Frameworks/Sparkle.framework")
		message(STATUS "Looking for Sparkle Framwork - found")
		#need to check a few things for autoupdate
		# this will set the UPPERCASE Variable to the right path 
		# for soft/Hard linknig of the framwork.
		# OPTION( VARIABLE "Description" Initial state)
		OPTION( USE_SPARKLE "Enable Sparkle Framwork on mac" ON )

		find_library(SPARKLE_FRAMEWORK Sparkle)
		find_library(APPKIT_FRAMEWORK AppKit)

		# we need to copy the public key to check the updates
		set(PUBLIC_KEY_PATH "${CMAKE_CURRENT_SOURCE_DIR}/macos_specific/dsa_pub_songbookclient.pem")
		IF(EXISTS ${PUBLIC_KEY_PATH})
			message(STATUS "Looking for Public Key - found")
			SET_SOURCE_FILES_PROPERTIES(
				${PUBLIC_KEY_PATH}
				PROPERTIES
				MACOSX_PACKAGE_LOCATION Resources
			)
			set(PUBLIC_KEY ${PUBLIC_KEY_PATH})
		ELSE(EXISTS ${PUBLIC_KEY_PATH})
			message(WARNING "Looking for Public Key - not found")
			message(WARNING "${PUBLIC_KEY_PATH} not found Sparkle Framwork will NOT work and may event prevent application from launching. Please consider Disabling Sparkle Framwork, creating a pair of key for testing purposes")
		ENDIF(EXISTS ${PUBLIC_KEY_PATH})
		
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
		# Hack Custom command to copy sparkle framwork
		set(CCS1 mkdir   ${CMAKE_CURRENT_BINARY_DIR}/${SONGBOOK_CLIENT_APPLICATION_NAME}.app/Contents/Frameworks)
		set(CCS2 cp -R ${SPARKLE_FRAMEWORK} ${CMAKE_CURRENT_BINARY_DIR}/${SONGBOOK_CLIENT_APPLICATION_NAME}.app/Contents/Frameworks/Sparkle.framework)
		set(CCS3 echo "Note : Having the message : ERROR: no file at '/usr/lib/@loader_path@loader_path' is normal")
		set(CCS4 echo "It is due to Qt encountering references to Sparkle Framwork.")
		
	ELSE(EXISTS "/Library/Frameworks/Sparkle.framework")
		message(STATUS "Looking for Sparkle Framwork - not found")
		set(CCS1 echo "The application has been compiled without support for auto-updating, you will have to check for it manually")
	ENDIF(EXISTS "/Library/Frameworks/Sparkle.framework")
	
	add_executable( ${SONGBOOK_CLIENT_APPLICATION_NAME} MACOSX_BUNDLE ${SONGBOOK_CLIENT_SOURCES} 
		${SONGBOOK_CLIENT_MOCS} ${SONGBOOK_CLIENT_RESSOURCES} ${COMPILED_TRANSLATIONS}
		${SONGBOOK_CLIENT_UI_HDRS}
		${qtpropertyeditor_SRCS} ${qtpropertyeditor_MOC} ${qtpropertyeditor_RESOURCES}
		${PUBLIC_KEY}
		${CMAKE_CURRENT_SOURCE_DIR}/macos_specific/book.icns
		${CMAKE_CURRENT_SOURCE_DIR}/macos_specific/song.icns
		)
	add_custom_command( TARGET ${SONGBOOK_CLIENT_APPLICATION_NAME} POST_BUILD
		COMMAND mkdir ARGS -p ${CMAKE_CURRENT_BINARY_DIR}/${SONGBOOK_CLIENT_APPLICATION_NAME}.app/Contents/Resources/lang
		COMMAND cp ARGS ${MACOSX_BUNDLE_ICON_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${SONGBOOK_CLIENT_APPLICATION_NAME}.app/Contents/Resources
		COMMAND cp ARGS ${SOURCE_DIR}/lang/*.qm ${CMAKE_CURRENT_BINARY_DIR}/${SONGBOOK_CLIENT_APPLICATION_NAME}.app/Contents/Resources/lang
		# this for command are set if sparkle is activated and are ment to copy the framwork inside the bundle
		# and warn the user about some error message that might be printed afterward
		COMMAND ${CCS1}
		COMMAND ${CCS2}
		COMMAND ${CCS3}
		COMMAND ${CCS4}
                COMMAND cp ARGS ${SOURCE_DIR}/pythonqt/lib/*.dylib ${CMAKE_CURRENT_BINARY_DIR}/${SONGBOOK_CLIENT_APPLICATION_NAME}.app/Contents/Frameworks
		COMMAND macdeployqt ${CMAKE_CURRENT_BINARY_DIR}/${SONGBOOK_CLIENT_APPLICATION_NAME}.app
		)

	if(USE_SPARKLE)
		# we need to add SPARKLE and APPKIT as '-framework' at link time
		target_link_libraries(${SONGBOOK_CLIENT_APPLICATION_NAME} ${APPKIT_FRAMEWORK})
		target_link_libraries(${SONGBOOK_CLIENT_APPLICATION_NAME} ${SPARKLE_FRAMEWORK})
	endif(USE_SPARKLE)


	# genération d'une plist custom pour sparkle from a plist.in where @VARAIBLE@ are replaces by the content of VARAIBLE
	# we need this for having the default url to check updates
	set_target_properties( ${SONGBOOK_CLIENT_APPLICATION_NAME}	PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/macos_specific/Info.plist.in )

