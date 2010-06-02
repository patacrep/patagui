// Copyright (C) 2009,2010 Romain Goffe, Alexandre Dupas
//
// Songbook Creator is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// Songbook Creator is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
// MA  02110-1301, USA.
//******************************************************************************

#include "dialog-new-song.hh"
#include "mainwindow.hh"
#include <QLayout>
#include <QSpinBox>

CDialogNewSong::CDialogNewSong(CMainWindow* parent)
  : QDialog()
{
  m_parent = parent;
  m_title = "";
  m_artist =  "";
  m_nbColumns = 0;
  m_capo = 0;
  
  setModal(true);

  QLineEdit* titleEdit = new QLineEdit;
  QLineEdit* artistEdit = new QLineEdit;
  QLabel* titleLabel = new QLabel(tr("Title: "));
  QLabel* artistLabel = new QLabel(tr("Artist: "));

  QSpinBox* nbColumnsEdit = new QSpinBox;
  nbColumnsEdit->setRange(0,20);
  QSpinBox* capoEdit      = new QSpinBox;
  capoEdit->setRange(0,20);
  QLabel* nbColumnsLabel = new QLabel(tr("Number of columns: "));
  QLabel* capoLabel = new QLabel(tr("Capo: "));
  
  // Action buttons
  QDialogButtonBox * buttonBox = new QDialogButtonBox;
  QPushButton * buttonAccept = new QPushButton(tr("Ok"));
  QPushButton * buttonClose = new QPushButton(tr("Close"));
  buttonAccept->setDefault(true);
  buttonBox->addButton(buttonAccept, QDialogButtonBox::ActionRole);
  buttonBox->addButton(buttonClose, QDialogButtonBox::DestructiveRole);
 
  connect( buttonAccept, SIGNAL(clicked()), this, SLOT(accept()) );
  connect( buttonClose, SIGNAL(clicked()), this, SLOT(close()) );

  QGroupBox* requiredFieldsBox = new QGroupBox(tr("Required fields"));
  QGridLayout* requiredLayout = new QGridLayout;
  requiredLayout->addWidget(titleLabel,   0,0,1,1);
  requiredLayout->addWidget(titleEdit,  0,1,1,1);
  requiredLayout->addWidget(artistLabel,  1,0,1,1);
  requiredLayout->addWidget(artistEdit, 1,1,1,1);
  requiredFieldsBox->setLayout(requiredLayout);

  QGroupBox* optionalFieldsBox = new QGroupBox(tr("Optional fields"));
  QGridLayout* optionalLayout = new QGridLayout;
  optionalLayout->addWidget(nbColumnsLabel,   0,0,1,1);
  optionalLayout->addWidget(nbColumnsEdit,  0,1,1,1);
  optionalLayout->addWidget(capoLabel,  1,0,1,1);
  optionalLayout->addWidget(capoEdit, 1,1,1,1);
  optionalFieldsBox->setLayout(optionalLayout);
  
  QBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(requiredFieldsBox);
  layout->addWidget(optionalFieldsBox);
  layout->addWidget(buttonBox);

  setLayout(layout);

  //Connections
  connect(titleEdit,  SIGNAL(textChanged(QString)), this, SLOT(setTitle(QString)) );
  connect(artistEdit, SIGNAL(textChanged(QString)), this, SLOT(setArtist(QString)) );
  connect(nbColumnsEdit, SIGNAL(valueChanged(int)), this, SLOT(setNbColumns(int)) );
  connect(capoEdit, SIGNAL(valueChanged(int)), this, SLOT(setCapo(int)) );

  setWindowTitle(tr("New song"));
  setMinimumWidth(450);
  show();
}
//------------------------------------------------------------------------------
QString CDialogNewSong::title() const
{
  return m_title;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setTitle(QString ATitle )
{
  m_title = ATitle;
}
//------------------------------------------------------------------------------
QString CDialogNewSong::artist() const
{
  return m_artist;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setArtist(QString AArtist )
{
  m_artist = AArtist;
}

//------------------------------------------------------------------------------
int CDialogNewSong::nbColumns() const
{
  return m_nbColumns;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setNbColumns(int ANbColumns)
{
  m_nbColumns = ANbColumns;
}
//------------------------------------------------------------------------------
int CDialogNewSong::capo() const
{
  return m_capo;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setCapo(int ACapo)
{
  m_capo = ACapo;
}
