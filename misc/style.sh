#!/bin/sh
#Description: remove trailing spaces

sed -i 's/[ \t]*$//' src/*.*
sed -i 's/\sif(/ if (/g' src/*.*
sed -i 's/\sfor(/ for (/g' src/*.*
sed -i 's/\swhile(/ while (/g' src/*.*

