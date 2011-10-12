# Copyright 1999-2010 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# Author: Guillaume Bouchard <guillaume@apinc.org>


EAPI=3

DESCRIPTION="Client for patacrep songbooks
The songbook-client is an interface to build
 customized PDF songbooks with lyrics, guitar
  chords and lilypond sheets from patacrep songbook."
HOMEPAGE="http://www.patacrep.com"
EGIT_REPO_URI="http://github.com/crep4ever/songbook-client.git"

inherit git autotools eutils

LICENSE="GPL-2+"
SLOT="0"
KEYWORDS="amd64 x86"
IUSE="python lilypond texlive"

DEPEND="x11-libs/qt-core-4.6
	x11-libs/qt-gui
        x11-libs/qt-script
        dev-util/cmake-2.6
	app-arch/libarchive
       
        python?   ( dev-lang/python )
        texlive?  ( app-text/texlive[linguas_fr,extra] )
        lilypond? ( media-sound/lilypond )
        "

RDEPEND="${DEPEND}"

src_install () {
   emake DESTDIR="$D" install || die
}

