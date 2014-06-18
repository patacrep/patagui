> Unfortunately, patagui (formerly songbook-gui and songbook-client) is
> currently not maintained: it should not work with the current version of
> [patacrep](http://github.com/patacrep/patacrep) and
> [patadata](http://github.com/patacrep/patadata). These new
> development are not far ahead of a working version, but do not have time right
> now to update the gui.
> 
> We keep this project in mind, and we would like it to be maintained again
> (either by us or by you), but we do not have the time to do so...
> 
> However, latest versions ([core 3.7.2](https://github.com/patacrep/patacrep/releases/tag/patacrep_3.7.2) and [gui 0.7.9](https://github.com/patacrep/patagui/releases/tag/0.7.9)) should still work. Enjoy!

# Description
This is a Qt4/C++ client that allows to pick up songs from the patacrep songbook database to build personalized songbooks.

# Screenshot
![SbClient](http://www.patacrep.com/data/images/sbc.png)

# Songbook-client
* required packages: cmake, libqt4-dev, libarchive-dev, libhunspell-dev
* recommended packages: python, lilypond, texlive-base, texlive-lang-french, texlive-latex-extra, texlive-fonts-recommended
* build and run:

>     git clone git://github.com/crep4ever/songbook-client.git
>     cd songbook-client
>     make && sudo make install
>     songbook-client

# Documentation
* User manual: http://www.patacrep.com/data/documents/doc_en.pdf
  Sources: http://github.com/crep4ever/songbook-documentation.git
* Developer manual: http://www.patacrep.com/data/songbook-client/doc/
>     doxygen Doxyfile

# Contact & Forums
* http://www.patacrep.com
* crep@team-on-fire.com
