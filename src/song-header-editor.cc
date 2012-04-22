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
#include "diagram.hh"

#include <QBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QToolButton>
#include <QSpacerItem>

#include <QFileInfo>
#include <QFile>
#include <QPixmapCache>
#include <QPixmap>
#include <QFileDialog>

#include <QUrl>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>

#include <QDebug>

CSongHeaderEditor::CSongHeaderEditor(QWidget *parent)
  : QWidget(parent)
  , m_titleLineEdit(new QLineEdit(this))
  , m_artistLineEdit(new QLineEdit(this))
  , m_albumLineEdit(new QLineEdit(this))
  , m_languageComboBox(new QComboBox(this))
  , m_columnCountLineEdit(new QLineEdit(this))
  , m_capoLineEdit(new QLineEdit(this))
  , m_coverLabel(new CCoverDropArea(this))
  , m_songEditor()
  , m_addDiagramButton(0)
  , m_spacer(0)
{
  m_languageComboBox->addItem
    (QIcon::fromTheme("flag-en", QIcon(":/icons/songbook/22x22/flags/flag-en.png")), tr("English"));
  m_languageComboBox->addItem
    (QIcon::fromTheme("flag-fr", QIcon(":/icons/songbook/22x22/flags/flag-fr.png")), tr("French"));
  m_languageComboBox->addItem
    (QIcon::fromTheme("flag-es", QIcon(":/icons/songbook/22x22/flags/flag-es.png")), tr("Spanish"));
  m_languageComboBox->addItem
    (QIcon::fromTheme("flag-pt", QIcon(":/icons/songbook/22x22/flags/flag-pt.png")), tr("Portuguese"));

  connect(m_titleLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_artistLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_albumLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_languageComboBox, SIGNAL(currentIndexChanged(const QString&)),
          SLOT(onLanguageSelected(const QString&)));
  connect(m_columnCountLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_capoLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(onTextEdited(const QString&)));

  m_titleLineEdit->setMinimumWidth(150);
  m_artistLineEdit->setMinimumWidth(150);
  m_albumLineEdit->setMinimumWidth(150);

  QBoxLayout *additionalInformationLayout = new QHBoxLayout();
  additionalInformationLayout->setContentsMargins(1, 1, 1, 1);
  additionalInformationLayout->addWidget(m_languageComboBox);
  additionalInformationLayout->addWidget(m_columnCountLineEdit);
  additionalInformationLayout->addWidget(m_capoLineEdit);
  additionalInformationLayout->addStretch();

  QBoxLayout *songInformationLayout = new QVBoxLayout();
  songInformationLayout->addWidget(m_titleLineEdit);
  songInformationLayout->addWidget(m_artistLineEdit);
  songInformationLayout->addWidget(m_albumLineEdit);
  songInformationLayout->addLayout(additionalInformationLayout);
  songInformationLayout->addStretch();

  QBoxLayout *coverLayout = new QVBoxLayout();
  coverLayout->setContentsMargins(2, 2, 2, 2);
  coverLayout->addWidget(m_coverLabel);
  coverLayout->addStretch();

  m_diagramsLayout = new QHBoxLayout;

  QWidget* scroll = new QWidget;
  scroll->setLayout(m_diagramsLayout);
  QScrollArea* diagramsScrollArea = new QScrollArea;
  diagramsScrollArea->setWidget(scroll);
  diagramsScrollArea->setBackgroundRole(QPalette::Dark);
  diagramsScrollArea->setWidgetResizable(true);
  diagramsScrollArea->setMinimumWidth(350);
  diagramsScrollArea->setMinimumHeight(150);
  diagramsScrollArea->setMaximumHeight(160);

  setMaximumHeight(160);

  QBoxLayout *mainLayout = new QHBoxLayout;
  mainLayout->setContentsMargins(1, 1, 1, 1);
  mainLayout->addLayout(coverLayout);
  mainLayout->addLayout(songInformationLayout);
  mainLayout->addWidget(diagramsScrollArea);
  mainLayout->setStretchFactor(songInformationLayout, 1);
  mainLayout->setStretchFactor(diagramsScrollArea, 2);
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
  m_languageComboBox->setCurrentIndex(m_languageComboBox->findText
				      (QLocale::languageToString(song().locale.language()),
				       Qt::MatchContains));
  m_columnCountLineEdit->setText(QString::number(song().columnCount));
  m_capoLineEdit->setText(QString::number(song().capo));

  m_coverLabel->setSong(song());
  m_coverLabel->update();

  QString gtab;
  foreach (gtab, song().gtabs)
    {
      CDiagramWidget *diagram = new CDiagramWidget(gtab, GuitarChord);
      connect(diagram, SIGNAL(diagramCloseRequested()), SLOT(removeDiagram()));
      m_diagramsLayout->addWidget(diagram);
    }

  QString utab;
  foreach (utab, song().utabs)
    {
      CDiagramWidget *diagram = new CDiagramWidget(utab, UkuleleChord);
      connect(diagram, SIGNAL(diagramCloseRequested()), SLOT(removeDiagram()));
      m_diagramsLayout->addWidget(diagram);
    }

  addNewDiagramButton();
}

void CSongHeaderEditor::onLanguageSelected(const QString &text)
{
  song().locale = QLocale(Song::languageFromString(text), QLocale::AnyCountry);
}

void CSongHeaderEditor::onTextEdited(const QString &text)
{
  QLineEdit *currentLineEdit = qobject_cast< QLineEdit* >(sender());
  if (currentLineEdit == m_titleLineEdit)
    song().title = text;
  else if (currentLineEdit == m_artistLineEdit)
    song().artist = text;
  else if (currentLineEdit == m_albumLineEdit)
    song().album = text;
  else if (currentLineEdit == m_columnCountLineEdit)
    song().columnCount = text.toInt();
  else if (currentLineEdit == m_capoLineEdit)
    song().capo = text.toInt();
  else
    qWarning() << "CSongHeaderEditor::onTextEdited unknow sender";

  emit(contentsChanged());
}

const QImage & CSongHeaderEditor::cover()
{
  return m_coverLabel->cover();
}

void CSongHeaderEditor::addNewDiagramButton()
{
  if(m_addDiagramButton)
    {
      m_diagramsLayout->removeItem(m_spacer);
      delete m_addDiagramButton;
    }

  m_addDiagramButton = new QToolButton;
  m_addDiagramButton->setIcon(QIcon::fromTheme("list-add", QIcon(":/icons/tango/32x32/actions/list-add.png")));
  connect(m_addDiagramButton, SIGNAL(clicked()), this, SLOT(addDiagram()));
  m_diagramsLayout->addWidget(m_addDiagramButton);
  m_spacer = new QSpacerItem(500, 20, QSizePolicy::Ignored, QSizePolicy::MinimumExpanding);
  m_diagramsLayout->addSpacerItem(m_spacer);
}

void CSongHeaderEditor::addDiagram()
{
  CDiagramWidget *diagram = new CDiagramWidget("\\gtab{<name>}{<fret>:<strings>}", GuitarChord);
  if (diagram->editChord())
    {
      connect(diagram, SIGNAL(diagramCloseRequested()), SLOT(removeDiagram()));
      m_diagramsLayout->addWidget(diagram);
      addNewDiagramButton();
    }
  else
    delete diagram;
}

void CSongHeaderEditor::removeDiagram()
{
  CDiagramWidget *diagram = qobject_cast< CDiagramWidget* >(QObject::sender());
  if(diagram)
    {
      m_diagramsLayout->removeWidget(diagram);
      disconnect(diagram,0,0,0);
      diagram->setParent(0);
      //delete diagram;
    }
}

//------------------------------------------------------------------------------

CCoverDropArea::CCoverDropArea(QWidget *parent)
  : QLabel(parent)
{
  setMinimumSize(150,150);
  setFrameStyle(QFrame::Raised | QFrame::Panel);
  setLineWidth(3);
  setAlignment(Qt::AlignCenter);
  setAutoFillBackground(true);
  setAcceptDrops(true);
  setToolTip(tr("Click or drop image to change cover"));
  connect(this, SIGNAL(changed()), SLOT(update()));
  clear();
}

void CCoverDropArea::dragEnterEvent(QDragEnterEvent *event)
{
  setBackgroundRole(QPalette::Highlight);
  event->acceptProposedAction();
  emit changed(event->mimeData());
}

void CCoverDropArea::dragMoveEvent(QDragMoveEvent *event)
{
  event->acceptProposedAction();
}

void CCoverDropArea::dropEvent(QDropEvent *event)
{
  const QMimeData *mimeData = event->mimeData();

  if (mimeData->hasText())
    {
      QUrl url(mimeData->text());
      m_filename = url.toLocalFile().trimmed();
      update();
    }
  else
    qWarning() << tr("CCoverDropArea::dropEvent cannot display dropped data");

  setBackgroundRole(QPalette::Dark);
  event->acceptProposedAction();
}

void CCoverDropArea::dragLeaveEvent(QDragLeaveEvent *event)
{
  clear();
  event->accept();
}

void CCoverDropArea::clear()
{
  setBackgroundRole(QPalette::Dark);
  emit changed();
}

void CCoverDropArea::update()
{
  if ( song().coverPath.isEmpty() ||
       song().coverName.isEmpty() ||
       m_filename.isEmpty() )
    return;

  // display the cover art
  QFileInfo file = QFileInfo(m_filename);
  if(!file.exists())
    qWarning() << QString(tr("CCoverDropArea::update invalid cover file : %1").arg(m_filename));

  song().coverPath = file.absolutePath();
  song().coverName = file.baseName();
  QPixmap pixmap;
  if (!QPixmapCache::find(file.baseName()+"-full", &pixmap))
    {
      setCover(file.filePath());
      pixmap = QPixmap::fromImage(cover());
      QPixmapCache::insert(file.baseName()+"-full", pixmap);
    }
  setPixmap(pixmap);
}

void CCoverDropArea::selectCover()
{
  QString filename = QFileDialog::getOpenFileName(this,
						  tr("Select cover"),
                                                  song().coverPath,
                                                  tr("Images (*.jpg)"));

  if( !filename.isEmpty() && filename != m_filename )
    {
      m_filename = filename;
      update();
    }
}

void CCoverDropArea::mousePressEvent(QMouseEvent *event)
{
  setFrameStyle(QFrame::Sunken | QFrame::Panel);
}

void CCoverDropArea::mouseReleaseEvent(QMouseEvent *event)
{
  setFrameStyle(QFrame::Raised | QFrame::Panel);
  selectCover();
}

void CCoverDropArea::setSong(const Song & sg)
{
  m_song = sg;
  m_filename = QString("%1/%2.jpg").arg(song().coverPath).arg(song().coverName);
  emit changed();
}

Song & CCoverDropArea::song()
{
  return m_song;
}

const QImage & CCoverDropArea::cover()
{
  return m_cover;
}

void CCoverDropArea::setCover(const QImage &cover)
{
  if(cover.isNull())
    qWarning() << tr("CCoverDropArea::setCover invalid cover");

  m_cover = cover.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void CCoverDropArea::setCover(const QString &path)
{
  setCover(QImage(path));
}
