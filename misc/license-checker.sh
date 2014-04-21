#!/bin/bash
#
# Author: M. Bussonnier
# Date: 15/04/2011
# Descritpion: This script looks for *.cc and *.hh files in SRC_DIRECTORY and checks their licenses
#
# Copyright (C) 2009-2011, Romain Goffe <romain.goffe@gmail.com>
# Copyright (C) 2009-2011, Alexandre Dupas <alexandre.dupas@gmail.com>
# Copyright (C) 2009-2011, Matthias Bussonnier <bussonniermatthias@gmail.com>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301, USA.

args=`getopt hvqs: $*`
set -- $args

usage()
{
cat << EOF
Usage: $0 SRC_DIRECTORY
Description: check licenses for *.cc and *.hh in SRC_DIRECTORY"

This script run the test1 or test2 over a machine.

OPTIONS:
   -h      Show this message
   -q      quiet silence red output
   -v      Verbose print  files who passed test
   -s      root of project

default value will print partially passed test
EOF
}
VERBOSE=0
QUIET=0
while getopts “hvqs:” OPTION
do
     case $OPTION in
         h)
             usage
             exit 1
             ;;
		 s)
			 SDIR=$OPTARG
			;;
         v)
             VERBOSE=1
             ;;
         q)
             QUIET=1
             ;;
         ?)
             usage
             exit
             ;;
     esac
done
if [ -z $SDIR ]
then
	SDIR='.'
fi
BASEDIR=`dirname $0`

VERT="\\033[0;32m"
JAUNE="\\033[1;33m"
NORMAL="\\033[0;39m"
ROUGE="\\033[1;31m"
ROSE="\\033[1;35m"
BLEU="\\033[1;34m"
BLANC="\\033[0;02m"
BLANCLAIR="\\033[1;08m"
JAUNE="\\033[1;33m"
CYAN="\\033[1;36m"

BINCOPYRIGHT=debian/binaryCopyright
LICFILE="$BASEDIR/lics/lic.txt"
function echov {
	if [[ $VERBOSE == 1 ]]
	then
		echo -e $VERT$1$NORMAL
	fi
}
function echor {
	if [[ $QUIET == 0 ]]
	then
		echo -e "$ROUGE""$1""$NORMAL"
	fi
}
function includefile {
 return  $(diff $1 $2 |grep '^>' | wc -l)
}

function chf
{
	BD=$2
	LICS=$(ls $BD)
	for LIC in $LICS ; do
		if $(includefile $1  $BD$LIC)
		then
			ftype=$(basename $LIC .txt)
			echo -en $VERT${ftype}" \t:"$NORMAL
			return 0
		fi
	done

	echor "fail\t:"$NORMAL
	return 1
}
function checklicense {
	str=$(chf $1 'misc/copyr/')
	y=$?
	str=${str}$(chf $1 'misc/lics/')
	x=$?

	#		if [[ $VERBOSE == 1 || ($cp == 0 && $QUIET != 1) ]]
	notok=$(( $x || $y ))

	if [[ (( $VERBOSE == 1 && $notok == 0 )) || ($QUIET != 1 && $notok == 1) ]]
	then
		echo -e ${str}"$1"
	fi

	return $notok
}

badtextfiles=0
self=$0
cc=$(find $SDIR/src -name '*.cc' -type f )
hh=$(find $SDIR/src -name '*.hh' -type f )

for i in $self $cc $hh ; do
	if !(checklicense $i)
	then
		badtextfiles=$(expr $badtextfiles + 1)
	fi
done

if [[ $badtextfiles != 0 ]]
then
	echor '********************************************'
	echor '*   Some files are not properly licensed   *'
	echor '********************************************'
else
	echov '********************************************'
	echov '*         Everything seems alright         *'
	echov '********************************************'
fi
echo -en $NORMAL

###### fin du check pour les fichiers de code ######
badbinaryfiles=0

cd $SDIR
# some check that the license file is correct:
# first no white space
j=$(egrep ' ' $BINCOPYRIGHT |grep -v '#'|wc -l)
if [ $j != 0 ]
then
	echor "Le fichier de définition de license contient des espace, assurez vous qu'il ne contienne que des tabulations !"
	egrep ' ' $BINCOPYRIGHT|grep -v '#'
	exit -1;
fi

#second non double tbulation
j=$(egrep '[[:blank:]][[:blank:]]' $BINCOPYRIGHT |grep -v '#'|wc -l)
if [ $j != 0 ]
then
	echo -e $ROUGE "Le fichier de définition de license contient des doubles tabulations ! Merci de les retirer! Vérifiez les lignes suiventes :" $NORMAL
	egrep '[[:blank:]][[:blank:]]' $BINCOPYRIGHT |grep -v '#'
	exit -1;
fi

# let's check if the file is in the license file
icns=$(find icons -name '*' -type f)

for i in $icns ; do
	j=$(grep $i $BINCOPYRIGHT|wc -l)
	str=$(grep $i $BINCOPYRIGHT|cut -f1)
	if [[ $j -eq 0 || $str == 'License' || $str == 'Nothing' || $str == 'Inconnu' ]]
	then
		echor "Nothing : $i"
		badbinaryfiles=$(expr $badbinaryfiles + 1 );
	else
		echov "$(grep $i $BINCOPYRIGHT)"
	fi
done

nexfile=0
# let's check that every file in the license file exist
tf=$(cat $BINCOPYRIGHT|grep -v '#'|cut -f3)
for i in $tf ; do
	if [ ! -e $i ]
	then
		echor "fichier non existant : $i"
		nexfile=$(expr $nexfile + 1);
	fi
	if [ -d $i ]
	then
		echo -e $BLEU "Do you really think you can copyright a directory ?" $i
	fi

done
cd - > /dev/null
echo -e $BLEU "You have"
echo -e $BLEU "$badtextfiles source file(s) with an issue on the copyright and/or license"
echo -e $BLEU "$badbinaryfiles binary file(s) with no license or not appearing in the copyright file"
echo -e $BLEU "$nexfile non existing files listed in the copyright file" $NORMAL
sum=$(expr $badtextfiles + $badbinaryfiles + $nexfile)
exit $sum
