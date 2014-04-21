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
  : QProcess(parent)
  , m_program()
  , m_arguments()
  , m_startMessage(tr("Processing"))
  , m_successMessage(tr("Success"))
  , m_errorMessage(tr("Error"))
  , m_urlToOpen()
{
  connect(this, SIGNAL(started()), SLOT(onStarted()));
  connect(this, SIGNAL(finished(int, QProcess::ExitStatus)), SLOT(onFinished(int, QProcess::ExitStatus)));
  connect(this, SIGNAL(readyReadStandardOutput()), SLOT(readStandardOutput()));
  connect(this, SIGNAL(readyReadStandardError()), SLOT(readStandardError()));
}

CMakeSongbookProcess::~CMakeSongbookProcess()
{}

void CMakeSongbookProcess::execute()
{
  emit(aboutToStart());
  start(program(), arguments());
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

void CMakeSongbookProcess::readStandardOutput()
{
  QString standardOutput = QString(readAllStandardOutput().data());
  emit(readOnStandardOutput(standardOutput));
}

void CMakeSongbookProcess::readStandardError()
{
  QString standardError = QString(readAllStandardError().data());
  emit(readOnStandardError(standardError));
}

void CMakeSongbookProcess::onStarted()
{
  emit(message(startMessage(), 0));
}

void CMakeSongbookProcess::onFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
  Q_UNUSED(exitCode);
  if (exitStatus != QProcess::NormalExit)
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
      emit(error(QProcess::UnknownError));
    }

  if (!QDesktopServices::openUrl(urlToOpen()))
    {
      qWarning() << tr("Can't open [%1].").arg(urlToOpen().toLocalFile());
      emit(error(QProcess::UnknownError));
    }
}
