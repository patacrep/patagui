#!/bin/sh
#
cd $HOME/songbook-client
rm -rf build/
rm -rf debian/songbook-*
rm -f lang/*.qm
make distclean
make
dpkg-buildpackage
mv ../songbook-client*.deb .
rm -f ../songbook-client*.*


