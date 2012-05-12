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

#include "utils/lineedit.hh"

#include <QBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSpinBox>
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
  , m_titleLineEdit(new LineEdit(this))
  , m_artistLineEdit(new LineEdit(this))
  , m_albumLineEdit(new LineEdit(this))
  , m_languageComboBox(new QComboBox(this))
  , m_columnCountSpinBox(new QSpinBox(this))
  , m_capoSpinBox(new QSpinBox(this))
  , m_transposeSpinBox(new QSpinBox(this))
  , m_coverLabel(new CCoverDropArea(this))
  , m_songEditor()
  , m_addDiagramButton(0)
  , m_spacer(0)
{
  //do not translate combobox content since "english", "french" etc is used by LaTeX
  m_languageComboBox->addItem
    (QIcon::fromTheme("flag-en", QIcon(":/icons/songbook/22x22/flags/flag-en.png")), "English");
  m_languageComboBox->addItem
    (QIcon::fromTheme("flag-fr", QIcon(":/icons/songbook/22x22/flags/flag-fr.png")), "French");
  m_languageComboBox->addItem
    (QIcon::fromTheme("flag-es", QIcon(":/icons/songbook/22x22/flags/flag-es.png")), "Spanish");
  m_languageComboBox->addItem
    (QIcon::fromTheme("flag-pt", QIcon(":/icons/songbook/22x22/flags/flag-pt.png")), "Portuguese");
  m_languageComboBox->setToolTip(tr("Language"));

  QLabel *columnCountLabel = new QLabel(this);
  columnCountLabel->setPixmap(QIcon(":/icons/songbook/22x22/columns.png").pixmap(22,22));
  m_columnCountSpinBox->setToolTip(tr("Number of columns for the song"));
  m_columnCountSpinBox->setRange(1,3);

  QLabel *capoLabel = new QLabel(this);
  capoLabel->setPixmap(QIcon(":/icons/songbook/22x22/capo.png").pixmap(22,22));
  m_capoSpinBox->setToolTip(tr("Fret on which the capo should be put"));
  m_capoSpinBox->setRange(0,9);

  QLabel *transposeLabel = new QLabel(this);
  transposeLabel->setPixmap(QIcon(":/icons/songbook/22x22/transpose.png").pixmap(22,22));
  m_transposeSpinBox->setToolTip(tr("Number of half-steps by which chords are transposed"));
  m_transposeSpinBox->setRange(-14,14);

  m_titleLineEdit->setMinimumWidth(150);
  m_titleLineEdit->setToolTip(tr("Song title"));
  m_artistLineEdit->setMinimumWidth(150);
  m_artistLineEdit->setToolTip(tr("Artist"));
  m_albumLineEdit->setMinimumWidth(150);
  m_albumLineEdit->setToolTip(tr("Album"));

  connect(m_titleLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_artistLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_albumLineEdit, SIGNAL(textEdited(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_languageComboBox, SIGNAL(currentIndexChanged(const QString&)),
          SLOT(onIndexChanged(const QString&)));
  connect(m_columnCountSpinBox, SIGNAL(valueChanged(int)),
          SLOT(onValueChanged(int)));
  connect(m_capoSpinBox, SIGNAL(valueChanged(int)),
          SLOT(onValueChanged(int)));
  connect(m_transposeSpinBox, SIGNAL(valueChanged(int)),
          SLOT(onValueChanged(int)));
  connect(m_coverLabel, SIGNAL(coverChanged()),
          SLOT(onCoverChanged()));

  QBoxLayout *additionalInformationLayout = new QHBoxLayout();
  additionalInformationLayout->setContentsMargins(1, 1, 1, 1);
  additionalInformationLayout->addWidget(m_languageComboBox);
  additionalInformationLayout->addWidget(columnCountLabel);
  additionalInformationLayout->addWidget(m_columnCountSpinBox);
  additionalInformationLayout->addWidget(capoLabel);
  additionalInformationLayout->addWidget(m_capoSpinBox);
  additionalInformationLayout->addWidget(transposeLabel);
  additionalInformationLayout->addWidget(m_transposeSpinBox);
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

CSongEditor * CSongHeaderEditor::songEditor() const
{
  return m_songEditor;
}

void CSongHeaderEditor::setSongEditor(CSongEditor *songEditor)
{
  m_songEditor = songEditor;
  update();
}

LineEdit * CSongHeaderEditor::titleLineEdit() const
{
  return m_titleLineEdit;
}

LineEdit * CSongHeaderEditor::artistLineEdit() const
{
  return m_artistLineEdit;
}

void CSongHeaderEditor::update()
{
  if (song().title.isEmpty())
    m_titleLineEdit->setInactiveText(tr("Song title"));
  else
    m_titleLineEdit->setText(song().title);

  if (song().artist.isEmpty())
    m_artistLineEdit->setInactiveText(tr("Artist"));
  else
    m_artistLineEdit->setText(song().artist);

  if (song().album.isEmpty())
    m_albumLineEdit->setInactiveText(tr("Album"));
  else
    m_albumLineEdit->setText(song().album);

  m_languageComboBox->setCurrentIndex(m_languageComboBox->findText
				      (QLocale::languageToString(song().locale.language()),
				       Qt::MatchContains));
  m_columnCountSpinBox->setValue(song().columnCount);
  m_capoSpinBox->setValue(song().capo);
  m_transposeSpinBox->setValue(song().transpose);
  m_coverLabel->update();

  QString gtab;
  foreach (gtab, song().gtabs)
    {
      CDiagramWidget *diagram = new CDiagramWidget(gtab, GuitarChord);
      connect(diagram, SIGNAL(diagramCloseRequested()), SLOT(removeDiagram()));
      connect(diagram, SIGNAL(diagramChanged()), SLOT(onDiagramChanged()));
      m_diagramsLayout->addWidget(diagram);
    }

  QString utab;
  foreach (utab, song().utabs)
    {
      CDiagramWidget *diagram = new CDiagramWidget(utab, UkuleleChord);
      connect(diagram, SIGNAL(diagramCloseRequested()), SLOT(removeDiagram()));
      connect(diagram, SIGNAL(diagramChanged()), SLOT(onDiagramChanged()));
      m_diagramsLayout->addWidget(diagram);
    }

  addNewDiagramButton();
}

void CSongHeaderEditor::onIndexChanged(const QString &text)
{
  song().locale = QLocale(Song::languageFromString(text.toLower()), QLocale::AnyCountry);
  emit(languageChanged(song().locale));
  emit(contentsChanged());
}

void CSongHeaderEditor::onValueChanged(int value)
{
  QSpinBox *currentSpinBox = qobject_cast< QSpinBox* >(sender());
  if (currentSpinBox == m_columnCountSpinBox)
    {
      song().columnCount = value;
    }
  else if (currentSpinBox == m_capoSpinBox)
    {
      song().capo = value;
    }
  else if (currentSpinBox == m_transposeSpinBox)
    {
      song().transpose = value;
    }
  else
    {
      qWarning() << "CSongHeaderEditor::onValueChanged unknow sender";
      return;
    }

  emit(contentsChanged());
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
  else
    {
      qWarning() << "CSongHeaderEditor::onTextEdited unknow sender";
      return;
    }

  emit(contentsChanged());
}

void CSongHeaderEditor::onDiagramChanged()
{
  song().gtabs = QStringList();
  for(int i=0; i < m_diagramsLayout->count(); ++i)
    if (CDiagramWidget *diagram = qobject_cast< CDiagramWidget* >(m_diagramsLayout->itemAt(i)->widget()))
      {
	if (diagram->type() == GuitarChord)
	  song().gtabs << diagram->toString();
	else if (diagram->type() == UkuleleChord)
	  song().utabs << diagram->toString();
      }

  emit(contentsChanged());
}

void CSongHeaderEditor::onCoverChanged()
{
  songEditor()->setNewCover(true);
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
  CDiagramWidget *diagram = new CDiagramWidget("\\gtab{}{0:}", GuitarChord);
  connect(diagram, SIGNAL(diagramCloseRequested()), SLOT(removeDiagram()));
  connect(diagram, SIGNAL(diagramChanged()), SLOT(onDiagramChanged()));
  if (diagram->editChord())
    {
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
      onDiagramChanged();
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
  setBackgroundRole(QPalette::Dark);

  QPixmap pixmap;
  if(!QPixmapCache::find("cover-missing-full", &pixmap))
    {
      pixmap = QIcon::fromTheme("image-missing", QIcon(":/icons/tango/128x128/status/image-missing.png")).pixmap(128, 128);
      QPixmapCache::insert("cover-missing-full", pixmap);
    }
  setPixmap(pixmap);
  connect(this, SIGNAL(changed()), SLOT(update()));
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
  if(m_filename.isEmpty() && !song().coverPath.isEmpty() && !song().coverName.isEmpty())
    m_filename = QString("%1/%2.jpg").arg(song().coverPath).arg(song().coverName);

  // display the cover art
  QFileInfo file = QFileInfo(m_filename);
  if(file.exists())
    {
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
      emit(coverChanged());
    }
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

Song & CCoverDropArea::song()
{
  return qobject_cast<CSongHeaderEditor*>(QLabel::parent())->song();
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
