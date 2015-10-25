// Copyright (C) 2009-2011, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2011, Alexandre Dupas <alexandre.dupas@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.
//******************************************************************************
#ifndef __MAKE_SONGBOOK_PROCESS_HH__
#define __MAKE_SONGBOOK_PROCESS_HH__

#include <QString>
#include <QStringList>
#include <QUrl>
#include <QObject>
#include "PythonQt.h"
#include "songbook.hh"

/*!
  \file make-songbook-process.hh
  \class MakeSongbookProcess
  \brief MakeSongbookProcess calls songbook's external processes.

  Three processes can be called:

  \li the build command (make)
  \li the clean command (make clean)
  \li the cleanall command (make cleanall)

  These commands are defined in preferences.hh.
  For Linux platforms, it directly uses makefile (with or without arguments).
  For MacOS and Windows platforms, it uses specific scripts that are
  respectively located into windows/ and macos/ subdirectories of the songbook
  directory.
*/

class MakeSongbookProcess : public QObject
{
    Q_OBJECT
    //    Q_PROPERTY(QString songbook READ songbook WRITE setSongbook)
    //    Q_PROPERTY(QStringList datadirs READ datadirs WRITE setDatadirs)

    Q_PROPERTY(QString startMessage READ startMessage WRITE setStartMessage)
    Q_PROPERTY(
        QString successMessage READ successMessage WRITE setSuccessMessage)
    Q_PROPERTY(QString errorMessage READ errorMessage WRITE setErrorMessage)

    Q_PROPERTY(QUrl urlToOpen READ urlToOpen WRITE setUrlToOpen)

public slots:

    /*!
    Execute the process once all parameters have been set.
    The signal aboutToStart is emitted before the execution.
    \sa aboutToStart
  */
    void execute();

    /*!
    Set the songbook to compile.
    \sa execute
  */
    void setSongbook(CSongbook *songbook);

    /*!
    Set the datadirs
    \sa setSongbook
  */
    void setDatadirs(const QStringList &datadirs);

    /*!
    Add a datadir
    \sa setDatadirs
  */
    void addDatadir(const QString &datadir);

    /*!
    Sets the \a message that should displayed when the process starts.
    The message is displayed within the status bar.
    \sa startMessage, setSuccessMessage, setErrorMessage
  */
    void setStartMessage(const QString &message);

    /*!
    Sets the \a message that should displayed when the process successfully
    exits.
    The message is displayed within the status bar.
    \sa successMessage, setStartMessage, setErrorMessage
  */
    void setSuccessMessage(const QString &message);

    /*!
    Sets the \a message that should displayed when the process exits with
    failure.
    The message is displayed within the status bar.
    \sa errorMessage, setStartMessage, setSuccessMessage
  */
    void setErrorMessage(const QString &message);

    /*!
    Sets an \a url that should be opened after the process successfully exits.
    The message is displayed within the status bar.
    \sa urlToOpen
  */
    void setUrlToOpen(const QUrl &url);

    /*!
     Stop the build
   */
    void stopBuilding();

public:
    /// Constructor.
    MakeSongbookProcess(QObject *parent = 0);

    /// Destructor.
    virtual ~MakeSongbookProcess();

    /*!
    Return the songbook to compile.
    \sa setSongbook
  */
    const CSongbook *songbook() const;

    /*!
    Return the datadirs
    \sa setDatadirs,addDatadir
  */
    const QStringList &datadirs() const;

    /*!
    Returns the message that is displayed when the process starts.
    \sa setStartMessage
  */
    const QString &startMessage() const;

    /*! Returns the message that is displayed when the process successfully
    exits.
    \sa setSuccessMessage
  */
    const QString &successMessage() const;

    /*!
    Returns the message that is displayed when the process exits with failure.
    \sa setErrorMessage
  */
    const QString &errorMessage() const;

    /*! Returns the url that should be opened when the process successfully
    exits.
    \sa setUrlToOpen
  */
    const QUrl &urlToOpen() const;

    /*! Sets Working directory for process
  */
    void setWorkingDirectory(const QString &dir);

    /*! Test for Python
  */
    const bool testPython();

signals:
    /*!
    This signal is emitted before the process is executed.
    \sa execute
  */
    void aboutToStart();

    /*!
    Emits a \a message when the process starts and exits after a delay of \a
    timeout.
    \sa startMessage, successMessage, errorMessage
  */
    void message(const QString &message, int timeout);

    /*!
      Signel emitted when the process is finished
  */
    void finished();

private slots:

    /*!
    Emits start message.
    \sa startMessage \sa message
  */
    void onStarted();

    /*!
    Emits exit message and try opening the url if successful.
    \sa message, url, errorMessage, successMessage
  */
    void onFinished(int exitCode);

    void stdOut(QString string);

    void stdErr(QString string);

private:
    CSongbook *m_songbook;
    QStringList m_datadirs;
    QString m_startMessage;
    QString m_successMessage;
    QString m_errorMessage;

    QUrl m_urlToOpen;
    PythonQtObjectPtr pythonModule;
};

#endif // __MAKE_SONGBOOK_PROCESS_HH__
