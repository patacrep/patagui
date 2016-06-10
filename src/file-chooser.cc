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
#include <QFileDialog>
#include <QFileInfo>
#include <QLineEdit>
#include <QPushButton>
#include <QBoxLayout>
#include <QCompleter>
#include <QDirModel>
#include <QSettings>

FileChooser::FileChooser(QWidget *parent)
    : QWidget(parent)
    , m_lineEdit(0)
    , m_button(0)
    , m_caption(QCoreApplication::applicationName())
    , m_directory(QDir::homePath())
    , m_path("")
    , m_filter("")
    , m_options(0)
{
    m_lineEdit = new QLineEdit();
    QCompleter *completer = new QCompleter(this);
    completer->setModel(new QDirModel(completer));
    completer->setCompletionMode(QCompleter::InlineCompletion);
    m_lineEdit->setCompleter(completer);

    connect(m_lineEdit, SIGNAL(textChanged(const QString &)), this,
            SLOT(setPath(const QString &)));

    m_button = new QPushButton(tr("Browse"));
    connect(m_button, SIGNAL(clicked()), SLOT(browse()));

    QLayout *layout = new QHBoxLayout;
    layout->addWidget(m_lineEdit);
    layout->addWidget(m_button);
    // disable layout's margin to have a proper "one widget" appearance
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);
}

FileChooser::~FileChooser() {}

void FileChooser::browse()
{
    QString selection;
    if (options() & QFileDialog::ShowDirsOnly)
        selection =
            QFileDialog::getExistingDirectory(this, caption(), directory());
    else
        selection = QFileDialog::getOpenFileName(this, caption(), directory(),
                                                 filter(), 0, options());

    if (!selection.isEmpty())
        setPath(selection);
}

QFileDialog::Options FileChooser::options() const { return m_options; }

void FileChooser::setOptions(const QFileDialog::Options &opts)
{
    m_options = opts;
}

QString FileChooser::filter() const { return m_filter; }

void FileChooser::setFilter(const QString &filter) { m_filter = filter; }

QString FileChooser::caption() const { return m_caption; }

void FileChooser::setCaption(const QString &caption) { m_caption = caption; }

QString FileChooser::directory() const { return m_directory; }

void FileChooser::setDirectory(const QString &directory)
{
    m_directory = directory;
}

void FileChooser::setDirectory(const QDir &directory)
{
    m_directory = directory.absolutePath();
}

QString FileChooser::path() const { return m_path; }

void FileChooser::setPath(const QString &path)
{
    if (QString::compare(m_path, path, Qt::CaseSensitive) == 0)
        return;

    if (m_lineEdit->text() != path)
        m_lineEdit->setText(path);

    m_path = path;

    QFileInfo fileInfo(m_path);
    if (fileInfo.isDir())
        setDirectory(m_path);
    else
        setDirectory(fileInfo.dir());

    emit(pathChanged(m_path));
}
