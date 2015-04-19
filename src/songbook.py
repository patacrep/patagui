#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Import all required modules
import json
import locale
import os.path
import textwrap
import sys

# Import patacrep modules
from patacrep.build import SongbookBuilder, DEFAULT_STEPS
from patacrep import __version__
from patacrep import errors
import patacrep.encoding

# Define variables
songbook = None
basename = None

# Define locale according to user's parameters
def setLocale():
    try:
        locale.setlocale(locale.LC_ALL, '')
    except locale.Error as error:
        print("Locale Error")
        # Throw error

# Load songbook and setup datadirs
def setupSongbook(songbook_path,datadir):
    global songbook
    global basename
    basename = os.path.basename(songbook_path)[:-3]
    # Load songbook from sb file.
    try:
        with patacrep.encoding.open_read(songbook_path) as songbook_file:
            songbook = json.load(songbook_file)
        if 'encoding' in songbook:
            with patacrep.encoding.open_read(
                songbook_path,
                encoding=songbook['encoding']
                ) as songbook_file:
                songbook = json.load(songbook_file)
    except Exception as error: # pylint: disable=broad-except
        print("Loading Error")
        # Throw Exception

    # Gathering datadirs
    datadirs = []
    if 'datadir' in songbook:
        # .sg file
        if isinstance(songbook['datadir'], str):
            songbook['datadir'] = [songbook['datadir']]
        datadirs += [
            os.path.join(
                os.path.dirname(os.path.abspath(songbook_path)),
                path
                )
            for path in songbook['datadir']
            ]
    # Default value
    datadirs.append(os.path.dirname(os.path.abspath(songbook_path)))
    songbook['datadir'] = datadirs

def Test():
    print("Hello World!")
