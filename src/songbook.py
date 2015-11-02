#!/usr/bin/env python
# -*- coding: utf-8 -*-

# Import all required modules
import json
import locale
import os.path
import textwrap
import sys
import logging
import threading

# Import patacrep modules
from patacrep.build import SongbookBuilder, DEFAULT_STEPS
from patacrep import __version__
from patacrep import errors
import patacrep.encoding

# Expose C++ to local python
from PythonQt import *

# Define global variables
sb_builder = None
process = None
stopProcess = False
logging.basicConfig(level=logging.DEBUG)

# Define locale according to user's parameters
def setLocale():
    try:
        locale.setlocale(locale.LC_ALL, '')
    except locale.Error as error:
        print("Locale Error")
        # Throw error

# Test patacrep version
def testPatacrep():
    return patacrep.__version__

def message(text):
    CPPprocess.message(text,0)

# Load songbook and setup datadirs
def setupSongbook(songbook_path,datadir):
    setLocale()
    global sb_builder
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
        print("Error while loading file '{}'".format(songbook_path))
        print(error)
        # Throw Exception
        return

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
    try:
        sb_builder = SongbookBuilder(songbook, basename)
        sb_builder.unsafe = True
    except errors.SongbookError as error:
        print("Error in formation of Songbook Builder")
        # Deal with error

# Wrapper around buildSongbook that manages the threading part
def build(steps):
    global stopProcess
    global process
    stopProcess = False
    process = threading.Thread(target=buildSongbook, args=(steps,))
    try:
        #logger.info('Starting process')
        process.start()
    except threading.ThreadError as error:
        #logger.warning('process Error occured')
        message(error)

    period = 2
    while process.is_alive():
        if stopProcess:
            process.terminate()
            print("terminated")
        # Check in 2 seconds
        process.join(period)
    message("end build")

# Inner function that actually builds the songbook
def buildSongbook(steps):
    global sb_builder
    message("Inner Function Reached")
    sys.stdout.flush()
    try:
        for step in steps:
            message("Building songbook: " + step)
            sb_builder.build_steps([step])
        message("Building finished")
    except errors.SongbookError as error:
        message("Building error")
        # Call proper function in CPPprocess
        message(error)
        raise
    message("Exiting buildSongbook function")

def stopBuild():
    message("Terminating process")
    global stopProcess
    stopProcess = True