#!/bin/sh
#Description: remove trailing spaces

sed -i 's/[ \t]*$//' src/*.*
sed -i 's/if(/if (/g' src/*.*
sed -i 's/for(/for (/g' src/*.*
sed -i 's/while(/while (/g' src/*.*

