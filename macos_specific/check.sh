#!/bin/bash

#Author: Carreau (M. Bussonnier)
#Descritpion: assist script to deploy the app on mac

if [ $# -ne 1 ];
then
    echo "Usage: $0 BUILD_DIR"
    echo "     BUILD_DIR: build directory of the application"
    echo "                such as ../build"
    exit 1
fi;

if [ -d $build_dir ]; then
    echo "Did you check the version number of the application?"
    pause 5
    macdeployqt $build_dir/build/Songbook-Client.app/
    cp -r /Library/Frameworks/Sparkle.framework/ $build_dir/build/Songbook-Client.app/Contents/Frameworks/
    echo "Please check that the public key exists"
    ls ../build/Songbook-Client.app/Contents/Resources
else 
    echo "Cannot find build directory : $build_dir"
fi 

