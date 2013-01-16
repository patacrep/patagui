// Copyright (C) 2009-2012, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2012, Alexandre Dupas <alexandre.dupas@gmail.com>
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
#include "diagram-editor.hh"

#include "song.hh"
#include "diagram-area.hh"

#include <QFile>
#include <QScrollArea>
#include <QSettings>
#include <QDir>

#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QBoxLayout>
#include <QFormLayout>
#include <QDebug>


CDiagramEditor::CDiagramEditor(QWidget *parent)
  : QDialog(parent)
  , m_infoIconLabel(new QLabel(this))
  , m_messageLabel(new QLabel(this))
  , m_diagramArea(0)
  , m_chord(0)
{
  setWindowTitle(tr("Chord editor"));

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
						     QDialogButtonBox::Cancel |
						     QDialogButtonBox::Reset);

  connect(buttonBox, SIGNAL(accepted()), SLOT(checkChord()));
  connect(buttonBox, SIGNAL(rejected()), SLOT(close()));
  connect(buttonBox->button(QDialogButtonBox::Reset), SIGNAL(clicked()),
	  this, SLOT(reset()));

  QGroupBox *instrumentGroupBox = new QGroupBox(tr("Instrument"));
  m_guitar  = new QRadioButton(tr("Guitar"));
  m_ukulele = new QRadioButton(tr("Ukulele"));
  connect(m_guitar, SIGNAL(toggled(bool)), this, SLOT(onInstrumentChanged(bool)));
  connect(m_ukulele, SIGNAL(toggled(bool)), this, SLOT(onInstrumentChanged(bool)));

  QVBoxLayout *instrumentLayout = new QVBoxLayout;
  instrumentLayout->addWidget(m_guitar);
  instrumentLayout->addWidget(m_ukulele);
  instrumentLayout->addStretch(1);
  instrumentGroupBox->setLayout(instrumentLayout);

  m_nameLineEdit = new QLineEdit;
  m_nameLineEdit->setToolTip(tr("The chord name such as A&m for A-flat minor"));

  m_fretSpinBox = new QSpinBox;
  m_fretSpinBox->setToolTip(tr("Fret"));
  m_fretSpinBox->setRange(0,9);

  m_stringsLineEdit = new QLineEdit;
  m_stringsLineEdit->setToolTip(tr("Symbols for each string of the guitar from lowest pitch to highest:\n"
				   "  X: string is not to be played\n"
				   "  0: string is to be played open\n"
				   "  [1-9]: string is to be played on the given numbered fret."));

  QRegExp rx("[X\\d]+");
  QRegExpValidator *validator = new QRegExpValidator(rx, 0);
  m_stringsLineEdit->setValidator(validator);

  QIcon iconInfo = QIcon::fromTheme("dialog-information");
  m_infoIconLabel->setPixmap(iconInfo.pixmap(24,24));
  m_messageLabel->setWordWrap(true);

  QHBoxLayout *layoutInformation = new QHBoxLayout;
  layoutInformation->addWidget(m_infoIconLabel);
  layoutInformation->addWidget(m_messageLabel, 1);
  layoutInformation->addStretch();
  m_infoIconLabel->hide();
  m_messageLabel->hide();

  m_importantCheckBox = new QCheckBox(tr("Important diagram"));
  m_importantCheckBox->setToolTip(tr("Mark this diagram as important."));

  QFormLayout *chordLayout = new QFormLayout;
  chordLayout->addRow(tr("Name:"), m_nameLineEdit);
  chordLayout->addRow(tr("Fret:"), m_fretSpinBox);
  chordLayout->addRow(tr("Strings:"), m_stringsLineEdit);

  QSettings settings;
  settings.beginGroup("general");
  QString songbookDir(settings.value("songbookPath", QDir::homePath()).toString());
  settings.endGroup();

  QFile file(QString("%1/tex/chords.tex").arg(songbookDir));

  if (file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
      // list of standard diagrams
      m_diagramArea = new CDiagramArea(this);
      m_diagramArea->setReadOnly(true);
      m_diagramArea->setColumnCount(8);

      connect(m_nameLineEdit, SIGNAL(textChanged(const QString &)),
	      m_diagramArea, SLOT(setNameFilter(const QString &)));
      connect(m_stringsLineEdit, SIGNAL(textChanged(const QString &)),
	      m_diagramArea, SLOT(setStringsFilter(const QString &)));
      connect(m_diagramArea, SIGNAL(diagramClicked(CChord *)),
	      this, SLOT(setChord(CChord *)));

      QTextStream stream (&file);
      stream.setCodec("UTF-8");
      QString content = stream.readAll();
      file.close();

      // parse chords.tex for gtab/utab
      QString line;
      QStringList lines = content.split("\n");
      foreach (line, lines)
	if (line.contains("\\gtab") || line.contains("\\utab"))
	  m_diagramArea->addDiagram(line.simplified());
    }

  QBoxLayout *formLayout = new QVBoxLayout;
  formLayout->addWidget(instrumentGroupBox);
  formLayout->addLayout(chordLayout);
  formLayout->addWidget(m_importantCheckBox);
  formLayout->addStretch(1);

  QBoxLayout *contentLayout = new QHBoxLayout;
  contentLayout->addLayout(formLayout);
  if (m_diagramArea)
    {
      QScrollArea *scrollArea = new QScrollArea;
      scrollArea->setWidget(m_diagramArea);
      scrollArea->setBackgroundRole(QPalette::Base);
      scrollArea->setWidgetResizable(true);
      scrollArea->setMinimumWidth(440);
      scrollArea->setMinimumHeight(400);
      contentLayout->addWidget(scrollArea, 1);
    }

  QBoxLayout *mainLayout = new QVBoxLayout;
  mainLayout->addLayout(contentLayout);
  mainLayout->addLayout(layoutInformation);
  mainLayout->addWidget(buttonBox);
  setLayout(mainLayout);
}

CDiagramEditor::~CDiagramEditor()
{
}

QSize CDiagramEditor::sizeHint() const
{
  if (m_diagramArea)
    return QSize(500, 420);
  else
    return QDialog::sizeHint();
}

void CDiagramEditor::reset()
{
  m_guitar->setChecked(true);
  m_ukulele->setChecked(false);
  m_nameLineEdit->clear();
  m_stringsLineEdit->clear();
  m_fretSpinBox->setValue(0);
  m_importantCheckBox->setChecked(false);

  if (m_diagramArea)
    m_diagramArea->clearFilters();
}

void CDiagramEditor::setChord(CChord *chord)
{
  if (!chord)
    return;

  m_chord = chord;

  m_guitar->setChecked(chord->instrument() == CChord::Guitar);
  m_ukulele->setChecked(chord->instrument() == CChord::Ukulele);
  m_nameLineEdit->setText(chord->name());
  m_fretSpinBox->setValue(chord->fret().toInt());
  m_stringsLineEdit->setText(chord->strings());
  m_importantCheckBox->setChecked(chord->isImportant());

  connect(m_nameLineEdit, SIGNAL(textChanged(const QString &)),
	  m_chord, SLOT(setName(const QString &)));
  connect(m_fretSpinBox, SIGNAL(valueChanged(const QString &)),
	  m_chord, SLOT(setFret(const QString &)));
  connect(m_stringsLineEdit, SIGNAL(textChanged(const QString &)),
	  m_chord, SLOT(setStrings(const QString &)));
  connect(m_guitar, SIGNAL(toggled(bool)),
	  m_chord, SLOT(switchInstrument(bool)));
  connect(m_ukulele, SIGNAL(toggled(bool)),
	  m_chord, SLOT(switchInstrument(bool)));

  if (m_diagramArea)
    m_diagramArea->clearFilters();
}

CChord * CDiagramEditor::chord() const
{
  return m_chord;
}

bool CDiagramEditor::checkChord()
{
  m_infoIconLabel->show();
  m_messageLabel->show();
  QString css("QLineEdit{ border: 1px solid red; border-radius: 2px; }");

  if (m_nameLineEdit->text().isEmpty())
    {
      m_messageLabel->setText(tr("Choose a valid chord name"));
      m_nameLineEdit->setStyleSheet(css);
      return false;
    }
  m_nameLineEdit->setStyleSheet(QString());

  if ((m_guitar->isChecked()  && m_stringsLineEdit->text().length() != CChord::GuitarStringCount) ||
      (m_ukulele->isChecked() && m_stringsLineEdit->text().length() != CChord::UkuleleStringCount))
    {
      m_stringsLineEdit->setStyleSheet(css);
      m_messageLabel->setText(tr("The number of strings doesn't match the chosen instrument"));
      return false;
    }
  m_stringsLineEdit->setStyleSheet(QString());

  if (!chord()->isValid())
    qWarning() << tr("CDiagramEditor::checkChord() an invalid chord has been accepted: ") << chord()->toString();

  accept();
  return true;
}

void CDiagramEditor::onInstrumentChanged(bool checked)
{
  Q_UNUSED(checked);

  if (m_diagramArea)
    {
      m_diagramArea->clearFilters();

      if (m_guitar->isChecked())
	m_diagramArea->setTypeFilter(CChord::Guitar);
      else if (m_ukulele->isChecked())
	m_diagramArea->setTypeFilter(CChord::Ukulele);
    }

  // set strings max length according to instrument
  if (m_guitar->isChecked())
    m_stringsLineEdit->setMaxLength(CChord::GuitarStringCount);
  else
    m_stringsLineEdit->setMaxLength(CChord::UkuleleStringCount);
}
