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
#include "diagram-area.hh"
#include "library.hh"

#include "utils/lineedit.hh"

#include <QStackedLayout>
#include <QBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>

#include <QFileInfo>
#include <QFile>
#include <QPixmapCache>
#include <QPixmap>
#include <QFileDialog>
#include <QCompleter>

#include <QUrl>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDragLeaveEvent>
#include <QContextMenuEvent>
#include <QMenu>
#include <QAction>

#include <QDebug>

CSongHeaderEditor::CSongHeaderEditor(QWidget *parent)
  : QWidget(parent)
  , m_songEditor(0)
  , m_titleLineEdit(new LineEdit(this))
  , m_artistLineEdit(new LineEdit(this))
  , m_albumLineEdit(new LineEdit(this))
  , m_originalSongLineEdit(new LineEdit(this))
  , m_urlLineEdit(new LineEdit(this))
  , m_languageComboBox(new QComboBox(this))
  , m_columnCountSpinBox(new QSpinBox(this))
  , m_capoSpinBox(new QSpinBox(this))
  , m_transposeSpinBox(new QSpinBox(this))
  , m_coverLabel(new CCoverDropArea(this))
  , m_viewMode(FullViewMode)
{
  // full view
  // do not translate combobox content since "english", "french" etc is used by LaTeX
  m_languageComboBox->addItem
    (QIcon::fromTheme("flag-en", QIcon(":/icons/songbook/22x22/flags/flag-en.png")), "English");
  m_languageComboBox->addItem
    (QIcon::fromTheme("flag-fr", QIcon(":/icons/songbook/22x22/flags/flag-fr.png")), "French");
  m_languageComboBox->addItem
    (QIcon::fromTheme("flag-es", QIcon(":/icons/songbook/22x22/flags/flag-es.png")), "Spanish");
  m_languageComboBox->addItem
    (QIcon::fromTheme("flag-pt", QIcon(":/icons/songbook/22x22/flags/flag-pt.png")), "Portuguese");
  m_languageComboBox->addItem
    (QIcon::fromTheme("flag-it", QIcon(":/icons/songbook/22x22/flags/flag-it.png")), "Italian");
  m_languageComboBox->setToolTip(tr("Language"));

  QLabel *columnCountLabel = new QLabel(this);
  columnCountLabel->setPixmap(QIcon(":/icons/songbook/22x22/columns.png").pixmap(22,22));
  columnCountLabel->setToolTip(tr("Number of columns for the song"));
  m_columnCountSpinBox->setToolTip(tr("Number of columns for the song"));
  m_columnCountSpinBox->setRange(1,3);

  QLabel *capoLabel = new QLabel(this);
  capoLabel->setPixmap(QIcon(":/icons/songbook/22x22/capo.png").pixmap(22,22));
  capoLabel->setToolTip(tr("Fret on which the capo should be put"));
  m_capoSpinBox->setToolTip(tr("Fret on which the capo should be put"));
  m_capoSpinBox->setRange(0,9);

  QLabel *transposeLabel = new QLabel(this);
  transposeLabel->setPixmap(QIcon(":/icons/songbook/22x22/transpose.png").pixmap(22,22));
  transposeLabel->setToolTip(tr("Number of half-steps by which chords are transposed"));
  m_transposeSpinBox->setToolTip(tr("Number of half-steps by which chords are transposed"));
  m_transposeSpinBox->setRange(-14,14);

  m_titleLineEdit->setMinimumWidth(150);
  m_titleLineEdit->setToolTip(tr("Song title"));
  m_artistLineEdit->setMinimumWidth(150);
  m_artistLineEdit->setToolTip(tr("Artist"));
  m_albumLineEdit->setMinimumWidth(150);
  m_albumLineEdit->setToolTip(tr("Album"));
  m_originalSongLineEdit->setMinimumWidth(70);
  m_originalSongLineEdit->setToolTip(tr("Original song"));
  m_urlLineEdit->setMinimumWidth(150);
  m_urlLineEdit->setToolTip(tr("Artist website"));

  connect(m_titleLineEdit, SIGNAL(textChanged(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_artistLineEdit, SIGNAL(textChanged(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_albumLineEdit, SIGNAL(textChanged(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_originalSongLineEdit, SIGNAL(textChanged(const QString&)),
          SLOT(onTextEdited(const QString&)));
  connect(m_urlLineEdit, SIGNAL(textChanged(const QString&)),
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

  QBoxLayout *additionalSongFieldsLayout = new QHBoxLayout();
  additionalSongFieldsLayout->setContentsMargins(1, 1, 1, 1);
  additionalSongFieldsLayout->addWidget(m_originalSongLineEdit);
  additionalSongFieldsLayout->addWidget(m_urlLineEdit);

  QBoxLayout *songInformationLayout = new QVBoxLayout();
  songInformationLayout->addWidget(m_titleLineEdit);
  songInformationLayout->addWidget(m_artistLineEdit);
  songInformationLayout->addWidget(m_albumLineEdit);
  songInformationLayout->addLayout(additionalSongFieldsLayout);
  songInformationLayout->addLayout(additionalInformationLayout);
  songInformationLayout->addStretch();

  m_diagramArea = new CDiagramArea(this);
  m_diagramArea->setRowCount(1);
  m_diagramArea->setReadOnly(false);
  connect(m_diagramArea, SIGNAL(contentsChanged()),
          SLOT(onDiagramsChanged()));

  QScrollArea* diagramsScrollArea = new QScrollArea;
  diagramsScrollArea->setWidget(m_diagramArea);
  diagramsScrollArea->setBackgroundRole(QPalette::Dark);
  diagramsScrollArea->setWidgetResizable(true);

  QBoxLayout *toMiniViewLayout = new QVBoxLayout;
  QPushButton *toMiniViewButton = new QPushButton;
  toMiniViewButton->setFlat(true);
  toMiniViewButton->setToolTip(tr("Mini view mode"));
  toMiniViewButton->setIcon(QIcon(":/icons/songbook/48x48/fold.png"));
  connect(toMiniViewButton, SIGNAL(clicked()), this, SLOT(toggleView()));
  toMiniViewLayout->addWidget(toMiniViewButton);
  toMiniViewLayout->addStretch();

  QWidget *fullView = new QWidget;
  QBoxLayout *fullViewLayout = new QHBoxLayout;
  fullViewLayout->setContentsMargins(4, 0, 4, 0);
  fullViewLayout->addWidget(m_coverLabel);
  fullViewLayout->addLayout(songInformationLayout);
  fullViewLayout->addWidget(diagramsScrollArea, 1);
  fullViewLayout->addLayout(toMiniViewLayout);
  fullView->setLayout(fullViewLayout);

  QBoxLayout *toFullViewLayout = new QVBoxLayout;
  QPushButton *toFullViewButton = new QPushButton;
  toFullViewButton->setFlat(true);
  toFullViewButton->setToolTip(tr("Full view mode"));
  toFullViewButton->setIcon(QIcon(":/icons/songbook/48x48/unfold.png"));
  connect(toFullViewButton, SIGNAL(clicked()), this, SLOT(toggleView()));
  toFullViewLayout->addWidget(toFullViewButton);
  toFullViewLayout->addStretch();

  // mini view
  QWidget *miniView = new QWidget;
  QLabel *miniTitle = new QLabel(m_titleLineEdit->text());
  connect(m_titleLineEdit, SIGNAL(textChanged(const QString &)), miniTitle, SLOT(setText(const QString &)));
  QLabel *miniArtist = new QLabel(m_artistLineEdit->text());
  connect(m_artistLineEdit, SIGNAL(textChanged(const QString &)), miniArtist, SLOT(setText(const QString &)));
  QLabel *miniCover = new QLabel;
  connect(m_coverLabel, SIGNAL(miniCoverChanged(const QPixmap &)), miniCover, SLOT(setPixmap(const QPixmap &)));

  QBoxLayout *miniViewLayout = new QHBoxLayout;
  miniViewLayout->setContentsMargins(4, 0, 4, 0);
  miniViewLayout->addWidget(miniCover);
  miniViewLayout->addWidget(miniArtist);
  miniViewLayout->addWidget(new QLabel(" - "));
  miniViewLayout->addWidget(miniTitle);
  miniViewLayout->addStretch();
  miniViewLayout->addLayout(toFullViewLayout);
  miniView->setLayout(miniViewLayout);

  m_stackedLayout = new QStackedLayout;
  m_stackedLayout->setContentsMargins(0, 0, 0, 0);
  m_stackedLayout->addWidget(miniView);
  m_stackedLayout->addWidget(fullView);
  m_stackedLayout->setCurrentIndex(1);

  QVBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->setContentsMargins(0, 0, 0, 0);
  mainLayout->addLayout(m_stackedLayout);
  setLayout(mainLayout);
  setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Maximum);
}

CSongHeaderEditor::~CSongHeaderEditor()
{}


QSize CSongHeaderEditor::sizeHint() const
{
  return QSize(500, 150);
}

void CSongHeaderEditor::toggleView()
{
  if (m_viewMode == FullViewMode)
    {
      m_viewMode = MiniViewMode;
      m_stackedLayout->setCurrentIndex(0);
      setMaximumHeight(30);
    }
  else if (m_viewMode == MiniViewMode)
    {
      m_viewMode = FullViewMode;
      m_stackedLayout->setCurrentIndex(1);
      setMaximumHeight(150);
    }
}

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

void CSongHeaderEditor::setLibrary(CLibrary * library)
{
  QCompleter *artistCompleter = new QCompleter;
  artistCompleter->setModel(library->artistCompletionModel());
  artistCompleter->setCaseSensitivity(Qt::CaseInsensitive);
  m_artistLineEdit->setCompleter(artistCompleter);

  QCompleter *albumCompleter = new QCompleter;
  albumCompleter->setModel(library->albumCompletionModel());
  albumCompleter->setCaseSensitivity(Qt::CaseInsensitive);
  m_albumLineEdit->setCompleter(albumCompleter);
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

  if (song().originalSong.isEmpty())
    m_originalSongLineEdit->setInactiveText(tr("Original song"));
  else
    m_originalSongLineEdit->setText(song().originalSong);

  if (song().url.isEmpty())
    m_urlLineEdit->setInactiveText(tr("Artist website"));
  else
    m_urlLineEdit->setText(song().url);

  m_languageComboBox->setCurrentIndex(m_languageComboBox->findText
				      (QLocale::languageToString(song().locale.language()),
				       Qt::MatchContains));
  if (song().columnCount < 1)
    song().columnCount = 2;
  else if (song().columnCount > 3)
    song().columnCount = 3;

  m_columnCountSpinBox->setValue(song().columnCount);
  m_capoSpinBox->setValue(song().capo);
  m_transposeSpinBox->setValue(song().transpose);
  m_coverLabel->update();

  QString gtab;
  foreach (gtab, song().gtabs)
    {
      m_diagramArea->addDiagram(gtab, CDiagram::GuitarChord);
    }

  QString utab;
  foreach (utab, song().utabs)
    {
      m_diagramArea->addDiagram(utab, CDiagram::UkuleleChord);
    }
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
  else if (currentLineEdit == m_originalSongLineEdit)
    {
      song().originalSong = text;
    }
  else if (currentLineEdit == m_urlLineEdit)
    {
      song().url = text;
    }
  else
    {
      qWarning() << "CSongHeaderEditor::onTextEdited unknow sender";
      return;
    }

  emit(contentsChanged());
}

void CSongHeaderEditor::onDiagramsChanged()
{
  song().gtabs = QStringList();
  song().utabs = QStringList();
  foreach (CDiagramWidget *diagram, m_diagramArea->diagrams())
    {
      if (diagram->type() == CDiagram::GuitarChord)
	song().gtabs << diagram->toString();
      else if (diagram->type() == CDiagram::UkuleleChord)
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

//------------------------------------------------------------------------------

CCoverDropArea::CCoverDropArea(CSongHeaderEditor *parent)
  : QLabel(parent)
  , m_parent(parent)
{
  setMinimumSize(132,132);
  setMaximumSize(132,132);
  setFrameStyle(QFrame::Raised | QFrame::Panel);
  setLineWidth(3);
  setAlignment(Qt::AlignCenter);
  setAutoFillBackground(true);
  setAcceptDrops(true);
  setToolTip(tr("Click or drop image to change cover"));
  setBackgroundRole(QPalette::Dark);
  setCover(QImage());
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

  if (mimeData->hasUrls())
    {
      QUrl url(mimeData->urls()[0]);
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
  if (m_filename.isEmpty() && !song().coverPath.isEmpty() && !song().coverName.isEmpty())
    m_filename = QString("%1/%2.jpg").arg(song().coverPath).arg(song().coverName);

  // display the cover art
  QPixmap pixmap;
  QFileInfo file = QFileInfo(m_filename);
  if (file.exists())
    {
      song().coverPath = file.absolutePath();
      song().coverName = file.baseName();
      if (!QPixmapCache::find(file.baseName()+"-full", &pixmap))
	{
	  setCover(file.filePath());
	  pixmap = QPixmap::fromImage(cover());
	  QPixmapCache::insert(file.baseName()+"-full", pixmap);
	}
    }
  else
    {
      song().coverPath = QString();
      song().coverName = QString();
      if (!QPixmapCache::find("cover-missing-full", &pixmap))
	{
	  pixmap = QIcon::fromTheme("image-missing", QIcon(":/icons/tango/128x128/status/image-missing.png")).pixmap(115, 115);
	  QPixmapCache::insert("cover-missing-full", pixmap);
	}
    }
  setPixmap(pixmap);
  emit(coverChanged());
  emit(miniCoverChanged(pixmap.scaled(28,28)));
}

void CCoverDropArea::selectCover()
{
  QString filename = QFileDialog::getOpenFileName(this,
						  tr("Select cover"),
                                                  song().coverPath,
                                                  tr("Images (*.jpg)"));

  if ( !filename.isEmpty() && filename != m_filename )
    {
      m_filename = filename;
      update();
    }
}

void CCoverDropArea::mousePressEvent(QMouseEvent *event)
{
  Q_UNUSED(event);
  setFrameStyle(QFrame::Sunken | QFrame::Panel);
}

void CCoverDropArea::mouseReleaseEvent(QMouseEvent *event)
{
  Q_UNUSED(event);
  setFrameStyle(QFrame::Raised | QFrame::Panel);
  selectCover();
}


void CCoverDropArea::contextMenuEvent(QContextMenuEvent *event)
{
  QMenu *menu = new QMenu(this);
  QAction *action = new QAction(tr("Clear cover"), this);
  action->setStatusTip(tr("Remove the song's cover"));
  connect(action, SIGNAL(triggered()), SLOT(clearCover()));
  menu->addAction(action);
  menu->exec(event->globalPos());
  delete menu;
}

CSongHeaderEditor * CCoverDropArea::parent() const
{
  if (!m_parent)
    qWarning() << tr("CCoverDropArea:: invalid parent");
  return m_parent;
}

void CCoverDropArea::setParent(CSongHeaderEditor * parent)
{
  m_parent = parent;
}

Song & CCoverDropArea::song()
{
  return parent()->song();
}

const QImage & CCoverDropArea::cover()
{
  return m_cover;
}

void CCoverDropArea::clearCover()
{
  m_cover = QImage();
  m_filename = QString();
  song().coverPath = QString();
  song().coverName = QString();
  update();
}

void CCoverDropArea::setCover(const QImage &cover)
{
  if (!cover.isNull())
    m_cover = cover.scaled(115, 115, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

void CCoverDropArea::setCover(const QString &path)
{
  setCover(QImage(path));
}
