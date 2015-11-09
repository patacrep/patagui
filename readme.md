> Unfortunately, Patagui (formerly songbook-gui and songbook-client) is
> currently only slowly maintained: it only partially works with the latest version of
> [patacrep](http://github.com/patacrep/patacrep) and
> [patadata](http://github.com/patacrep/patadata).
>
> We keep this project in mind, and we would like it to be properly up to date but
> several new features (full switch to python3, integration of chordpro songs) require a lot
> of time to be able to keep the software up to date.
>
> However, latest versions ([core 3.7.2](https://github.com/patacrep/patacrep/releases/tag/patacrep_3.7.2) and [gui 0.7.9](https://github.com/patacrep/patagui/releases/tag/0.7.9)) should still fully work. Enjoy!

[![Build status](https://ci.appveyor.com/api/projects/status/yydomf8riq5m3o7j?svg=true)](https://ci.appveyor.com/project/LaTruelle/patagui)

# Description
This is a Qt5/C++/Python client that allows to pick up songs from the patacrep songbook database to build personalized songbooks.

# Screenshot
![SbClient](http://www.patacrep.com/data/images/sbc.png)

# Patagui
* required packages: cmake, libqt5-dev, libarchive-dev, libhunspell-dev, python3
* recommended packages: lilypond, texlive-base, texlive-lang-french, texlive-latex-extra, texlive-fonts-recommended
* build and run:

>     git clone git://github.com/patacrep/patagui.git
>     cd patagui
>     make && sudo make install
>     Patagui

# Documentation
* User manual: http://www.patacrep.com/data/documents/doc_en.pdf
  Sources: http://github.com/patacrep/patacrep-doc.git
* Developer manual: http://www.patacrep.com/data/songbook-client/doc/
>     doxygen Doxyfile

# Contact & Forums
* http://www.patacrep.com
* crep@team-on-fire.com
