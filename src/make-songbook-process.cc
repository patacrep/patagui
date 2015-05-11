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

#include "make-songbook-process.hh"

#include <QDesktopServices>
#include <QFile>

#include <QDebug>

CMakeSongbookProcess::CMakeSongbookProcess(QObject *parent)
    : QObject(parent)
    , m_program()
    , m_arguments()
    , m_startMessage(tr("Processing"))
    , m_successMessage(tr("Success"))
    , m_errorMessage(tr("Error"))
    , m_urlToOpen()
{
    // Setup Python interpreter
    PythonQt::init(PythonQt::RedirectStdOut);
    pythonModule = PythonQt::self()->getMainModule();
    // Redirect Std Out and Std Err to qDebug output through slot for easier debug.
    connect(PythonQt::self(), SIGNAL(pythonStdOut(QString)), SLOT(stdOut(QString)));
    connect(PythonQt::self(), SIGNAL(pythonStdErr(QString)), SLOT(stdOut(QString)));
    // Import Python file containing all necessary functions and imports
    pythonModule.evalFile(":/python_scripts/songbook.py");
}

CMakeSongbookProcess::~CMakeSongbookProcess()
{}

void CMakeSongbookProcess::stdOut(QString string)
{
    qDebug() << string;
}

void CMakeSongbookProcess::execute()
{
    emit(aboutToStart());
    pythonModule.evalScript("setupSongbook('/Users/emmanuel/aa.sb','/Users/emmanuel/Documents/Logiciels/patacrep/patacrep/data/examples/songs/')");
    pythonModule.evalScript("build(['tex'])");
    emit(message("Finished Execution",0));
}

void CMakeSongbookProcess::setCommand(const QString &command)
{
    QStringList args = command.split(" ");
    setProgram(args[0]);
    args.pop_front();
    setArguments(args);
}

void CMakeSongbookProcess::setProgram(const QString &program)
{
    m_program = program;
}

void CMakeSongbookProcess::setArguments(const QStringList &arguments)
{
    m_arguments = arguments;
}

void CMakeSongbookProcess::setStartMessage(const QString &message)
{
    m_startMessage = message;
}

void CMakeSongbookProcess::setSuccessMessage(const QString &message)
{
    m_successMessage = message;
}

void CMakeSongbookProcess::setErrorMessage(const QString &message)
{
    m_errorMessage = message;
}

void CMakeSongbookProcess::setUrlToOpen(const QUrl &urlToOpen)
{
    m_urlToOpen = urlToOpen;
}

void CMakeSongbookProcess::setWorkingDirectory(const QString &dir)
{
    // TODO Do Something ?
}

QString CMakeSongbookProcess::command() const
{
    QString commandString = QString("%1 %2").arg(program()).arg(arguments().join(" "));
    return commandString;
}

const QString & CMakeSongbookProcess::program() const
{
    return m_program;
}

const QStringList & CMakeSongbookProcess::arguments() const
{
    return m_arguments;
}

const QString & CMakeSongbookProcess::startMessage() const
{
    return m_startMessage;
}

const QString & CMakeSongbookProcess::successMessage() const
{
    return m_successMessage;
}

const QString & CMakeSongbookProcess::errorMessage() const
{
    return m_errorMessage;
}

const QUrl & CMakeSongbookProcess::urlToOpen() const
{
    return m_urlToOpen;
}

void CMakeSongbookProcess::onStarted()
{
    emit(message(startMessage(), 0));
}

void CMakeSongbookProcess::onFinished(int exitCode)
{
    if (exitCode != 0)
    {
        emit(message(errorMessage(), 0));
        return;
    }

    emit(message(successMessage(), 0));
    if (urlToOpen().isEmpty() || !urlToOpen().isValid())
        return;

    emit(message(tr("Opening %1.").arg(urlToOpen().toString()), 1000));
    if (!QFile(urlToOpen().toLocalFile()).exists())
    {
        qWarning() << tr("File [%1] does not exist.").arg(urlToOpen().toLocalFile());
    }
    if (!QDesktopServices::openUrl(urlToOpen()))
    {
        qWarning() << tr("Can't open [%1].").arg(urlToOpen().toLocalFile());
    }
}
