#!/bin/sh
#Description: remove trailing spaces

sed -i 's/[ \t]*$//' src/*.*
