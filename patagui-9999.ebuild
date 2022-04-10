# Copyright 1999-2010 Gentoo Foundation
# Distributed under the terms of the GNU General Public License v2
# Author: Guillaume Bouchard <guillaume@apinc.org>


EAPI=3

DESCRIPTION="Client for patacrep songbooks
Patagui is an interface to build customized
PDF songbooks with lyrics, guitar chords and
lilypond sheets from patacrep songbook."
HOMEPAGE="http://www.patacrep.fr"
EGIT_REPO_URI="http://github.com/patacrep/patagui.git"

inherit git autotools eutils

LICENSE="GPL-2+"
SLOT="0"
KEYWORDS="amd64 x86"
IUSE="python lilypond texlive"

DEPEND="x11-libs/qt-core-5.4
	      x11-libs/qt-gui
        dev-util/cmake-3.0
	      app-arch/libarchive
        dev-lang/python-3.3

        texlive?  ( app-text/texlive[linguas_fr,extra] )
        lilypond? ( media-sound/lilypond )
        "

RDEPEND="${DEPEND}"

src_install () {
   emake DESTDIR="$D" install || die
}

