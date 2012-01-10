MacOs specific instructions

# Compiling

## Requirements
 - Cmake
 - Qt libraries : note that the downloable Qt framework is supposed to work with Objective-C, not C/C++ directly.
   Be carefull not to have a QT4 version installed through macport.
   otherwise you **should imperatively** check that `which qmake` does not
   point to the macport version !

If it is the case, remove all occurence of /opt/* from your path before 
running (C)Make, or it will cache the Library location !

## Options

 - Sparkle : enable auto-updates. The Sparkle framework should be installed in /Library/Framework

 - Libarchive : enable library download from interface. //macport ?


run
    > $ make

to build the application in the "build" subdirectory

# Misc

- There is no Xcode project.
- The application have only been tested on OS X 10.6.x

# Packaging 

Once compilation is completed :

  - bump application version
  - check for the public key in contents ressources
  - build dmg
  - mount dmg
  - check dmg background
  - generate the key to sign the application and move into appcast
  - upload
