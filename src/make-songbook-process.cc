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

MakeSongbookProcess::MakeSongbookProcess(QObject *parent)
    : QObject(parent)
    , m_startMessage(tr("Processing"))
    , m_successMessage(tr("Success"))
    , m_errorMessage(tr("Error"))
    , m_urlToOpen()
{
    // Setup Python interpreter
    PythonQt::init(PythonQt::RedirectStdOut);
    pythonModule = PythonQt::self()->getMainModule();
    // Redirect Std Out and Std Err to qDebug output through slot for easier
    // debug.
    connect(PythonQt::self(), SIGNAL(pythonStdOut(QString)),
            SLOT(stdOut(QString)));
    connect(PythonQt::self(), SIGNAL(pythonStdErr(QString)),
            SLOT(stdErr(QString)));
    // Import Python file containing all necessary functions and imports
    pythonModule.evalFile(":/python_scripts/songbook.py");
}

MakeSongbookProcess::~MakeSongbookProcess() {}

void MakeSongbookProcess::stdOut(QString string)
{
    // Hack to simplify output
    if (!string.simplified().isEmpty()) {
        emit message(string.simplified(), 0);
    }
}

void MakeSongbookProcess::stdErr(QString string)
{
    // Hack to simplify output
    if (!string.simplified().isEmpty()) {
        emit message("PY: " + string.simplified(), 0);
        // qWarning() << string.simplified().toUtf8().constData();
    }
}

void MakeSongbookProcess::execute()
{
    emit(aboutToStart());
    if (!m_songbook->filename().isEmpty()) {
        // Expose Songbook to python
        pythonModule.addObject("songbook", m_songbook);
        pythonModule.addObject("CPPprocess", this);
        pythonModule.evalScript("setupSongbook(songbook.filename,'" +
                                m_datadirs.first() + "')");
        pythonModule.evalScript("build(['tex', 'pdf', 'sbx', 'pdf', 'clean'])");
        // pythonModule.removeVariable("songbook");
        emit(message("Finished Execution", 0));
        emit(finished());
    } else {
        emit(message("Error: no songbook loaded", 0));
        emit(finished());
    }
}

const bool MakeSongbookProcess::testPython()
{
    QVariant patacrepVersion = pythonModule.call("testPatacrep");
    if (patacrepVersion.toString() == "4.0.0") {
        emit message("Patacrep Found", 0);
        return true;
    } else {
        emit message("Patacrep Not Found", 0);
        return false;
    }
}

void MakeSongbookProcess::setSongbook(CSongbook *songbook)
{
    m_songbook = songbook;
}

void MakeSongbookProcess::setDatadirs(const QStringList &datadirs)
{
    m_datadirs.append(datadirs);
}

void MakeSongbookProcess::addDatadir(const QString &datadir)
{
    m_datadirs.append(datadir);
}

void MakeSongbookProcess::setStartMessage(const QString &message)
{
    m_startMessage = message;
}

void MakeSongbookProcess::setSuccessMessage(const QString &message)
{
    m_successMessage = message;
}

void MakeSongbookProcess::setErrorMessage(const QString &message)
{
    m_errorMessage = message;
}

void MakeSongbookProcess::setUrlToOpen(const QUrl &urlToOpen)
{
    m_urlToOpen = urlToOpen;
}

void MakeSongbookProcess::setWorkingDirectory(const QString &dir)
{
    pythonModule.evalScript("os.chdir('" + dir + "')");
}

void MakeSongbookProcess::stopBuilding()
{
    pythonModule.evalScript("stopBuild()");
}

const CSongbook *MakeSongbookProcess::songbook() const { return m_songbook; }

const QString &MakeSongbookProcess::startMessage() const
{
    return m_startMessage;
}

const QString &MakeSongbookProcess::successMessage() const
{
    return m_successMessage;
}

const QString &MakeSongbookProcess::errorMessage() const
{
    return m_errorMessage;
}

const QUrl &MakeSongbookProcess::urlToOpen() const { return m_urlToOpen; }

void MakeSongbookProcess::onStarted() { emit(message(startMessage(), 0)); }

void MakeSongbookProcess::onFinished(int exitCode)
{
    if (exitCode != 0) {
        emit(message(errorMessage(), 0));
        return;
    }

    emit(message(successMessage(), 0));
    if (urlToOpen().isEmpty() || !urlToOpen().isValid())
        return;

    emit(message(tr("Opening %1.").arg(urlToOpen().toString()), 1000));
    if (!QFile(urlToOpen().toLocalFile()).exists()) {
        qWarning() << tr("File [%1] does not exist.")
                          .arg(urlToOpen().toLocalFile());
    }
    if (!QDesktopServices::openUrl(urlToOpen())) {
        qWarning() << tr("Can't open [%1].").arg(urlToOpen().toLocalFile());
    }
}
