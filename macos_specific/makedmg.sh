#! /bin/bash
# this script need
# https://github.com/Carreau/yoursway-create-dmg
# to correctly create a .dmg
if [ -z "$1" ]; then dmg_name="Patagui v0.X.x.dmg" ; else dmg_name="$1"; fi
rm -rf .tmp
test -f $dmg_name && rm $dmg_name
echo "using "$dmg_name" as image disk name"
~/yoursway-create-dmg/create-dmg   \
	--icon-size 96  \
	--linktoappfolder \
	--window-pos 413 295  \
	--window-size 770 367  \
	--icon Applications 527 187 \
	--volname "Patagui v0.5.1"   \
	--icon "Patagui.app" 201 187  \
	--background ./macos_specific/image_disque_backgroud.png  \
	$dmg_name ~/patagui/build/Patagui.app
