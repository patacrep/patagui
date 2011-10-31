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
#include "file-chooser.hh"

#include <QApplication>
#include <QFileInfo>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QSettings>

CFileChooser::CFileChooser()
  : QWidget()
  , m_lineEdit(0)
  , m_button(0)
  , m_dialog(new QFileDialog)
  , m_caption(QCoreApplication::applicationName())
{
  m_lineEdit = new QLineEdit();
  connect(m_lineEdit, SIGNAL(textChanged(const QString &)),
	  this, SLOT(setPath(const QString &)));

  m_button = new QPushButton(tr("Browse"));
  connect(m_button, SIGNAL(clicked()), SLOT(browse()));

  QLayout *layout = new QHBoxLayout;
  layout->addWidget(m_lineEdit);
  layout->addWidget(m_button);
  // disable layout's margin to have a proper "one widget" appeareance
  layout->setContentsMargins(0, 0, 0, 0);
  setLayout(layout);

  readSettings();
}

CFileChooser::~CFileChooser()
{
  writeSettings();
}

void CFileChooser::readSettings()
{
  QSettings settings;
  settings.beginGroup("dialog");
  QByteArray state = settings.value("state", QByteArray()).toByteArray();
  QString dir  = settings.value("directory", QDir::homePath()).toString();
  QString path  = settings.value("path", dir).toString();
  settings.endGroup();

  dialog()->restoreState(state);
  setDirectory(dir);
  setPath(path);
}

void CFileChooser::writeSettings()
{
  QSettings settings;
  settings.beginGroup("dialog");
  settings.setValue("state", dialog()->saveState());
  settings.setValue("directory", directory().absolutePath());
  settings.setValue("path", path());
  settings.endGroup();
}

void CFileChooser::browse()
{
  if(dialog()->exec() == QDialog::Accepted)
    {
      QStringList pathList = dialog()->selectedFiles();
      if (!pathList.isEmpty())
	setPath(pathList[0]);
    }
}

QFileDialog::AcceptMode CFileChooser::acceptMode() const
{
  return dialog()->acceptMode();
}

void CFileChooser::setAcceptMode(const QFileDialog::AcceptMode &mode)
{
  dialog()->setAcceptMode(mode);
}

QFileDialog::Options CFileChooser::options() const
{
  return dialog()->options();
}

void CFileChooser::setOptions(const QFileDialog::Options &opts)
{
  dialog()->setOptions(opts);
}

QFileDialog::FileMode CFileChooser::fileMode() const
{
  return dialog()->fileMode();
}

void CFileChooser::setFileMode(const QFileDialog::FileMode &mode)
{
  dialog()->setFileMode(mode);
}

void CFileChooser::setFilter(const QString &filter)
{
  dialog()->setNameFilter(filter);
}

QString CFileChooser::caption() const
{
  return dialog()->windowTitle();
}

void CFileChooser::setCaption(const QString &caption)
{
  dialog()->setWindowTitle(caption);
}

QDir CFileChooser::directory() const
{
  return dialog()->directory();
}

void CFileChooser::setDirectory(const QString &directory)
{
  dialog()->setDirectory(directory);
}

void CFileChooser::setDirectory(const QDir &directory)
{
  dialog()->setDirectory(directory);
}

QString CFileChooser::path() const
{
  return m_lineEdit->text();
}

void CFileChooser::setPath(const QString &path)
{
  if( QString::compare(this->path(), path, Qt::CaseSensitive) )
    m_lineEdit->setText(path);

  QFileInfo fileInfo(path);
  if (fileInfo.isDir())
    setDirectory(path);
  else
    setDirectory(fileInfo.dir());

  emit(pathChanged(path));
}

QFileDialog * CFileChooser::dialog() const
{
  return m_dialog;
}
