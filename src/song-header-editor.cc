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

#include "song-header-editor.hh"

#include "song-editor.hh"

#include <QBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>

#include <QFileInfo>
#include <QFile>
#include <QPixmapCache>
#include <QPixmap>

#include <QDebug>

CSongHeaderEditor::CSongHeaderEditor(QWidget *parent)
  : QWidget(parent)
  , m_titleLineEdit(new QLineEdit(this))
  , m_artistLineEdit(new QLineEdit(this))
  , m_albumLineEdit(new QLineEdit(this))
  , m_languageLineEdit(new QLineEdit(this))
  , m_columnCountLineEdit(new QLineEdit(this))
  , m_capoLineEdit(new QLineEdit(this))
  , m_coverLabel(new QLabel(this))
  , m_songEditor()
{
  connect(m_titleLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_artistLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_albumLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_languageLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_columnCountLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_capoLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(onTextEdited(const QString&)));

  QBoxLayout *additionalInformationLayout = new QHBoxLayout();
  additionalInformationLayout->setContentsMargins(1, 1, 1, 1);
  additionalInformationLayout->addWidget(m_languageLineEdit);
  additionalInformationLayout->addWidget(m_columnCountLineEdit);
  additionalInformationLayout->addWidget(m_capoLineEdit);

  QFormLayout *songInformationLayout = new QFormLayout();
  songInformationLayout->addRow(tr("Title: "), m_titleLineEdit);
  songInformationLayout->addRow(tr("Artist: "), m_artistLineEdit);
  songInformationLayout->addRow(tr("Album: "), m_albumLineEdit);
  songInformationLayout->addRow(additionalInformationLayout);

  QFrame *coverFrame = new QFrame(this);
  coverFrame->setFrameShape(QFrame::StyledPanel);
  coverFrame->setFrameShadow(QFrame::Raised);
  QBoxLayout *coverLayout = new QHBoxLayout();
  coverLayout->setContentsMargins(1, 1, 1, 1);
  coverLayout->addWidget(m_coverLabel);
  coverFrame->setLayout(coverLayout);

  QBoxLayout *mainLayout = new QHBoxLayout();
  mainLayout->setContentsMargins(1, 1, 1, 1);
  mainLayout->addWidget(coverFrame);
  mainLayout->addLayout(songInformationLayout);
  setLayout(mainLayout);
}

CSongHeaderEditor::~CSongHeaderEditor()
{}

Song & CSongHeaderEditor::song()
{
  return songEditor()->song();
}

CSongEditor * CSongHeaderEditor::songEditor()
{
  return m_songEditor;
}

void CSongHeaderEditor::setSongEditor(CSongEditor *songEditor)
{
  m_songEditor = songEditor;
  update();
}

void CSongHeaderEditor::update()
{
  m_titleLineEdit->setText(song().title);
  m_artistLineEdit->setText(song().artist);
  m_albumLineEdit->setText(song().album);
  m_languageLineEdit->setText(QLocale::languageToString(song().locale.language()));
  m_columnCountLineEdit->setText(QString::number(song().columnCount));
  m_capoLineEdit->setText(QString::number(song().capo));

  // display the cover art
  m_coverLabel->setMinimumSize(QSize(128,128));
  QFileInfo file = QFileInfo(QString("%1/%2.jpg").arg(song().coverPath).arg(song().coverName));
  if (file.exists())
    {
      QPixmap pixmap;
      if (!QPixmapCache::find(file.baseName()+"-full", &pixmap))
        {
          setCover(file.filePath());
          pixmap = QPixmap::fromImage(cover());
          QPixmapCache::insert(file.baseName()+"-full", pixmap);
        }
      m_coverLabel->setPixmap(pixmap);
    }
}

void CSongHeaderEditor::onTextEdited(const QString &text)
{
  QLineEdit *currentLineEdit = qobject_cast< QLineEdit* >(sender());
  if (currentLineEdit == m_titleLineEdit)
    {
      song().title = text;
    }
  else if (currentLineEdit == m_artistLineEdit)
    {
      song().artist = text;
    }
  else if (currentLineEdit == m_albumLineEdit)
    {
      song().album = text;
    }
  else if (currentLineEdit == m_languageLineEdit)
    {
      song().locale = QLocale(Song::languageFromString(text), QLocale::AnyCountry);
    }
  else if (currentLineEdit == m_columnCountLineEdit)
    {
      song().columnCount = text.toInt();
    }
  else if (currentLineEdit == m_capoLineEdit)
    {
      song().capo = text.toInt();
    }
  emit(contentsChanged());
}

const QImage & CSongHeaderEditor::cover()
{
  return m_cover;
}

void CSongHeaderEditor::setCover(const QImage &cover)
{
  m_cover = cover.scaled(128, 128, Qt::KeepAspectRatio);
}

void CSongHeaderEditor::setCover(const QString &path)
{
  setCover(QImage(path));
}
