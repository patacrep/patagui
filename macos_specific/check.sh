#!/bin/bash
	echo ' Avez vous vérifiez le numéro de version ?' 
	pause 5
	macdeployqt ../build/Songbook-Client.app/
	cp -r /Library/Frameworks/Sparkle.framework/ ../build/Songbook-Client.app/Contents/Frameworks/
	echo ' Vérifiez la présence de la clé publique '
	ls ../build/Songbook-Client.app/Contents/Resources
