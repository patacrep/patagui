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
#include <QString>

#include <QDebug>

CMakeSongbookProcess::CMakeSongbookProcess(QObject *parent)
    : QObject(parent)
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
    connect(PythonQt::self(), SIGNAL(pythonStdErr(QString)), SLOT(stdErr(QString)));
    // Import Python file containing all necessary functions and imports
    pythonModule.evalFile(":/python_scripts/songbook.py");
}

CMakeSongbookProcess::~CMakeSongbookProcess()
{}

void CMakeSongbookProcess::stdOut(QString string)
{
    // Hack to simplify output
    if (!string.simplified().isEmpty()) {
        emit message(string.simplified(),0);
    }
}

void CMakeSongbookProcess::stdErr(QString string)
{
    // Hack to simplify output
    if (!string.simplified().isEmpty()) {
        emit message("PY: " + string.simplified(),0);
        // qWarning() << string.simplified().toUtf8().constData();
    }
}

void CMakeSongbookProcess::execute()
{
    emit(aboutToStart());
    if (!m_songbook->filename().isEmpty()) {
        // pythonModule.evalScript(QString("setupSongbook('%1','%2')").arg(m_songbook,m_datadirs.first()));
        // pythonModule.evalScript("build(['tex', 'pdf', 'sbx', 'pdf', 'clean'])");
        // Expose Songbook to python
        pythonModule.addObject("songbook", m_songbook);
        pythonModule.addObject("CPPprocess", this);
        pythonModule.evalScript("print(songbook.filename)");
        pythonModule.evalScript("setupSongbook(songbook.filename,'" + m_datadirs.first() + "')");
        pythonModule.evalScript("build(['tex', 'pdf', 'sbx', 'pdf', 'clean'])");
        // pythonModule.removeVariable("songbook");
        emit(message("Finished Execution",0));
        emit(finished());
    }
    else{
        emit(message("Error: no songbook loaded",0));
        emit(finished());
    }
}

void CMakeSongbookProcess::setSongbook(CSongbook *songbook)
{
    m_songbook = songbook;
}

void CMakeSongbookProcess::setDatadirs(const QStringList &datadirs)
{
    m_datadirs.append(datadirs);
}

void CMakeSongbookProcess::addDatadir(const QString &datadir)
{
    m_datadirs.append(datadir);
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
    pythonModule.evalScript("os.chdir('" + dir + "')");
}

void CMakeSongbookProcess::stopBuilding()
{
    pythonModule.evalScript("stopBuild()");
}

const CSongbook* CMakeSongbookProcess::songbook() const
{
    return m_songbook;
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
