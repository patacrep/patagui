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
  void execute();

  void setCommand(const QString &command);
  void setProgram(const QString &program);
  void setArguments(const QStringList &arguments);
  void setStartMessage(const QString &message);
  void setSuccessMessage(const QString &message);
  void setErrorMessage(const QString &message);

  void setUrlToOpen(const QUrl &url);

public:
  CMakeSongbookProcess(QObject *parent = 0);
  virtual ~CMakeSongbookProcess();

  QString command() const;
  const QString & program() const;
  const QStringList & arguments() const;

  const QString & startMessage() const;
  const QString & successMessage() const;
  const QString & errorMessage() const;

  const QUrl & urlToOpen() const;

signals:
  void aboutToStart();
  void message(const QString &message, int timeout);
  void readOnStandardOutput(const QString &output);
  void readOnStandardError(const QString &error);

private slots:
  void readStandardOutput();
  void readStandardError();
  void onStarted();
  void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
  QString m_program;
  QStringList m_arguments;

  QString m_startMessage;
  QString m_successMessage;
  QString m_errorMessage;

  QUrl m_urlToOpen;
};

#endif // __MAKE_SONGBOOK_PROCESS_HH__
