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

#include <QProcess>
#include <QString>
#include <QStringList>
#include <QUrl>

/*!
  \file make-songbook-process.hh
  \class CMakeSongbookProcess
  \brief CMakeSongbookProcess calls songbook's external processes.

  Three processes can be called:

  \li the build command (make)
  \li the clean command (make clean)
  \li the cleanall command (make cleanall)

  These commands are defined in preferences.hh.
  For Linux platforms, it directly uses makefile (with or without arguments).
  For MacOS and Windows platforms, it uses specific scripts that are respectively located
  into windows/ and macos/ subdirectories of the songbook directory.
*/

class CMakeSongbookProcess : public QProcess
{
    Q_OBJECT
    Q_PROPERTY(QString command READ command WRITE setCommand)
    Q_PROPERTY(QString program READ program WRITE setProgram)
    Q_PROPERTY(QStringList arguments READ arguments WRITE setArguments)

    Q_PROPERTY(QString startMessage READ startMessage WRITE setStartMessage)
    Q_PROPERTY(QString successMessage READ successMessage WRITE setSuccessMessage)
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
    Sets the \a command that should be executed.
    For instance: "make songbook.pdf".
    \sa command, program, arguments
  */
    void setCommand(const QString &command);

    /*!
    Sets the \a message that should displayed when the process starts.
    The message is displayed within the status bar.
    \sa startMessage, setSuccessMessage, setErrorMessage
  */
    void setStartMessage(const QString &message);

    /*!
    Sets the \a message that should displayed when the process successfully exits.
    The message is displayed within the status bar.
    \sa successMessage, setStartMessage, setErrorMessage
  */
    void setSuccessMessage(const QString &message);

    /*!
    Sets the \a message that should displayed when the process exits with failure.
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

public:
    /// Constructor.
    CMakeSongbookProcess(QObject *parent = 0);

    /// Destructor.
    virtual ~CMakeSongbookProcess();

    /*!
    Returns the whole command to be executed.
    \sa setCommand
  */
    QString command() const;

    /*!
    Returns the message that is displayed when the process starts.
    \sa setStartMessage
  */
    const QString & startMessage() const;

    /*! Returns the message that is displayed when the process successfully exits.
    \sa setSuccessMessage
  */
    const QString & successMessage() const;

    /*!
    Returns the message that is displayed when the process exits with failure.
    \sa setErrorMessage
  */
    const QString & errorMessage() const;

    /*! Returns the url that should be opened when the process successfully exits.
    \sa setUrlToOpen
  */
    const QUrl & urlToOpen() const;

signals:
    /*!
    This signal is emitted before the process is executed.
    \sa execute
  */
    void aboutToStart();

    /*!
    Emits a \a message when the process starts and exits after a delay of \a timeout.
    \sa startMessage, successMessage, errorMessage
  */
    void message(const QString &message, int timeout);

    /*!
    This signal is emitted when information is available on standard output.
    \sa readOnStandardError
  */
    void readOnStandardOutput(const QString &output);

    /*!
    This signal is emitted when information is available on error output.
    \sa readOnStandardOutput
  */
    void readOnStandardError(const QString &error);

private slots:
    /*!
    Reads information from standard output.
    \sa readStandardError
  */
    void readStandardOutput();

    /*!
    Reads information from error output.
    \sa readStandardOutput
  */
    void readStandardError();

    /*!
    Emits start message.
    \sa startMessage \sa message
  */
    void onStarted();

    /*!
    Emits exit message and try opening the url if successful.
    \sa message, url, errorMessage, successMessage
  */
    void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

    void stdOut(QString string);

private:

    /*!
    Returns the name of the process.
    \sa setProgram, arguments, setArguments, command, setCommand
  */
    const QString & program() const;

    /*!
    Sets the \a program name of the process.
    The name corresponds to the first world of the command.
    \sa program, command, setCommand
  */
    void setProgram(const QString &program);

    /*!
    Returns the arguments of the process.
    Arguments are the words that comes after the program name in the command.
    \sa setArguments, command, setCommand
  */
    const QStringList & arguments() const;

    /*!
    Sets the \a arguments of the process.
    Arguments are the words that comes after the program name in the command.
    \sa arguments, command, setCommand
  */
    void setArguments(const QStringList &arguments);

    QString m_program;
    QStringList m_arguments;

    QString m_startMessage;
    QString m_successMessage;
    QString m_errorMessage;

    QUrl m_urlToOpen;
};

#endif // __MAKE_SONGBOOK_PROCESS_HH__
