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

#include "diagram.hh"

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
{
  setWindowTitle(tr("Chord editor"));

  QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok |
						     QDialogButtonBox::Cancel);

  connect(buttonBox, SIGNAL(accepted()), SLOT(checkChord()));
  connect(buttonBox, SIGNAL(rejected()), SLOT(close()));
  
  QGroupBox *instrumentGroupBox = new QGroupBox(tr("Instrument"));
  m_guitar  = new QRadioButton(tr("Guitar"));
  m_ukulele = new QRadioButton(tr("Ukulele"));

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
  m_stringsLineEdit->setMaxLength(CDiagram::GuitarStringCount);
  m_stringsLineEdit->setToolTip(tr("Symbols for each string of the guitar from lowest pitch to highest:\n"
				 "  X: string is not to be played\n"
				 "  0: string is to be played open\n"
				 "  [1-9]: string is to be played on the given numbered fret."));
  QRegExp rx("[X\\d]+");
  QRegExpValidator validator(rx, 0);
  m_stringsLineEdit->setValidator(&validator);

  QIcon iconInfo = QIcon::fromTheme("dialog-information");
  m_infoIconLabel->setPixmap(iconInfo.pixmap(24,24));

  QHBoxLayout *layoutInformation = new QHBoxLayout;
  layoutInformation->addWidget(m_infoIconLabel);
  layoutInformation->addWidget(m_messageLabel);
  m_infoIconLabel->hide();
  m_messageLabel->hide();

  m_importantCheckBox = new QCheckBox(tr("Important diagram"));
  m_importantCheckBox->setToolTip(tr("Mark this diagram as important."));

  QFormLayout *chordLayout = new QFormLayout;
  chordLayout->addRow(tr("Name:"), m_nameLineEdit);
  chordLayout->addRow(tr("Fret:"), m_fretSpinBox);
  chordLayout->addRow(tr("Strings:"), m_stringsLineEdit);

  QBoxLayout *layout = new QVBoxLayout;
  layout->addWidget(instrumentGroupBox);
  layout->addLayout(chordLayout);
  layout->addWidget(m_importantCheckBox);
  layout->addLayout(layoutInformation);
  layout->addWidget(buttonBox);
  setLayout(layout);
}

CDiagramEditor::~CDiagramEditor()
{
}

QString CDiagramEditor::chordName() const
{
  return m_nameLineEdit->text();
}

QString CDiagramEditor::chordStrings() const
{
  return m_stringsLineEdit->text();
}

QString CDiagramEditor::chordFret() const
{
  return (m_fretSpinBox->value() == 0) ? 
    "" : QString::number(m_fretSpinBox->value());
}

bool CDiagramEditor::isChordImportant() const
{
  return m_importantCheckBox->isChecked();
}

void CDiagramEditor::setDiagram(CDiagram *diagram)
{
  m_guitar->setChecked(diagram->type() == CDiagram::GuitarChord);
  m_ukulele->setChecked(diagram->type() == CDiagram::UkuleleChord);
  m_nameLineEdit->setText(diagram->chord());
  m_fretSpinBox->setValue(diagram->fret().toInt());
  m_stringsLineEdit->setText(diagram->strings());
  m_importantCheckBox->setChecked(diagram->isImportant());
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

  if ((m_guitar->isChecked()  && m_stringsLineEdit->text().length() != CDiagram::GuitarStringCount) ||
      (m_ukulele->isChecked() && m_stringsLineEdit->text().length() != CDiagram::UkuleleStringCount))
    {
      m_stringsLineEdit->setStyleSheet(css);
      m_messageLabel->setText(tr("The number of strings doesn't match the chosen instrument"));
      return false;
    }
  m_stringsLineEdit->setStyleSheet(QString());

  accept();
  return true;
}
