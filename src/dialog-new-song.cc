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
#include "utils/utils.hh"
#include "mainwindow.hh"

CDialogNewSong::CDialogNewSong(CMainWindow* AParent)
  : QDialog()
  ,m_parent(AParent)
  ,m_workingPath()
  ,m_title()
  ,m_artist()
  ,m_nbColumns(2)
  ,m_capo(0)
{
  m_workingPath = parent()->workingPath();
  connect(parent(), SIGNAL(workingPathChanged(QString)),
	  this, SLOT(setWorkingPath(QString)));

  //Required fields
  //title
  QLabel* titleLabel = new QLabel(tr("Title: "));
  m_titleEdit = new QLineEdit;

  //artist
  QLabel* artistLabel = new QLabel(tr("Artist: "));
  m_artistEdit = new QLineEdit;

  //Optional fields
  //album
  QLabel* albumLabel = new QLabel(tr("Album: "));
  QLineEdit* albumEdit = new QLineEdit;

  //cover
  QLabel* coverLabel = new QLabel(tr("Cover: "));
  m_coverEdit = new QLineEdit(QString());
  QPushButton *browseCoverButton = new QPushButton(tr("Browse"));

  //lang
  QLabel* langLabel = new QLabel(tr("Language: "));
  QComboBox* langComboBox = new QComboBox;
  langComboBox->addItem ("english");
  langComboBox->addItem ("french");
  langComboBox->addItem ("spanish");
  setLang(langComboBox->currentText());

  //nb columns
  QLabel* nbColumnsLabel = new QLabel(tr("Columns: "));
  QSpinBox* nbColumnsEdit = new QSpinBox;
  nbColumnsEdit->setValue(m_nbColumns);
  nbColumnsEdit->setRange(0,10);

  //capo
  QLabel* capoLabel = new QLabel(tr("Capo: "));
  QSpinBox* capoEdit      = new QSpinBox;
  capoEdit->setRange(0,20);

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
  requiredLayout->addWidget(m_titleEdit,  0,1,1,1);
  requiredLayout->addWidget(artistLabel,  1,0,1,1);
  requiredLayout->addWidget(m_artistEdit, 1,1,1,1);
  requiredFieldsBox->setLayout(requiredLayout);

  QGroupBox* optionalFieldsBox = new QGroupBox(tr("Optional fields"));
  QGridLayout* optionalLayout = new QGridLayout;
  optionalLayout->addWidget(albumLabel,   0,0,1,1);
  optionalLayout->addWidget(albumEdit,  0,1,1,5);
  optionalLayout->addWidget(coverLabel,   1,0,1,1);
  optionalLayout->addWidget(m_coverEdit,  1,1,1,3);
  optionalLayout->addWidget(browseCoverButton,  1,4,1,2);
  optionalLayout->addWidget(langLabel,   2,0,1,1);
  optionalLayout->addWidget(langComboBox,  2,1,1,1);
  optionalLayout->addWidget(nbColumnsLabel,   2,2,1,1);
  optionalLayout->addWidget(nbColumnsEdit,  2,3,1,1);
  optionalLayout->addWidget(capoLabel,  2,4,1,1);
  optionalLayout->addWidget(capoEdit, 2,5,1,1);
  optionalFieldsBox->setLayout(optionalLayout);

  QBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(requiredFieldsBox);
  layout->addWidget(optionalFieldsBox);
  layout->addWidget(buttonBox);

  //Connections
  connect(m_titleEdit,  SIGNAL(textChanged(QString)), this, SLOT(setTitle(QString)) );
  connect(m_artistEdit, SIGNAL(textChanged(QString)), this, SLOT(setArtist(QString)) );
  connect(langComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setLang(const QString&)) );
  connect(nbColumnsEdit, SIGNAL(valueChanged(int)), this, SLOT(setNbColumns(int)) );
  connect(capoEdit, SIGNAL(valueChanged(int)), this, SLOT(setCapo(int)) );
  connect(albumEdit, SIGNAL(textChanged(QString)), this, SLOT(setAlbum(QString)) );
  connect(browseCoverButton, SIGNAL(clicked()), this, SLOT(browseCover()) );
  connect(m_coverEdit, SIGNAL(textChanged(QString)), this, SLOT(setCover(QString)) );

  connect(this, SIGNAL(accepted()), this, SLOT(accept()) );

  setLayout(layout);
  setWindowTitle(tr("New song"));
  setMinimumWidth(450);
  show();
}
//------------------------------------------------------------------------------
CDialogNewSong::~CDialogNewSong()
{
  delete m_coverEdit;
}
//------------------------------------------------------------------------------
void CDialogNewSong::accept()
{
  addSong();
}
//------------------------------------------------------------------------------
bool CDialogNewSong::checkRequiredFields()
{
  bool result = true;
  QMap<QLineEdit*, QString> map;
  map.insert(m_titleEdit, title());
  map.insert(m_artistEdit, artist());
  QMapIterator<QLineEdit*, QString> it(map);
    
  while(it.hasNext())
    {
      it.next();
      if(it.value().isEmpty())
	{
	  it.key()->setStyleSheet("border: 1px solid red;border-radius: 3px;");
	  result = false;
	}
      else
	{
	  it.key()->setStyleSheet(QString());
	}
    }
  return result;
}
//------------------------------------------------------------------------------
QString CDialogNewSong::songTemplate()
{
  QString text;;
  text.append(QString("\\selectlanguage{%1}\n").arg(lang()));
  if (nbColumns() > 0)
    text.append(QString("\\songcolumns{%1}\n").arg(nbColumns()));
  
  text.append(QString("\\beginsong{%1}[by=%2").arg(title()).arg(artist()));
  
  if(!cover().isEmpty())
    text.append(QString(",cov=%1").arg(SbUtils::stringToFilename(album(),"-")));

  if(!album().isEmpty())
    text.append(QString(",album=%1").arg(album()));
 
  text.append(QString("]\n\n"));
  
  if(!cover().isEmpty())
    text.append(QString("\\cover\n"));

  if (capo() > 0)
    text.append(QString("\\capo{%1}\n").arg(capo()));
  
  text.append(QString("\n\\endsong"));

  return text;
}
//------------------------------------------------------------------------------
void CDialogNewSong::addSong()
{
  if ( !checkRequiredFields() )
    return;

  //make new dir
  QString dirpath = QString("%1/songs/%2").arg(workingPath()).arg(SbUtils::stringToFilename(artist(),"_"));
  QString filepath = QString("%1/%2.sg").arg(dirpath).arg(SbUtils::stringToFilename(title(),"_"));
  QDir dir(dirpath);

  if (!dir.exists())
    dir.mkpath(dirpath);

  //handle album and cover
  if( SbUtils::copyFile(cover(), dirpath) && !album().isEmpty() )
    { 
      QFile copy(QString("%1/%2").arg(dirpath).arg(QFileInfo(cover()).fileName()));
      QString convertedFileName(QString("%1.jpg").arg(SbUtils::stringToFilename(album(),"-")));
      if( !copy.rename(QString("%1/%2").arg(dirpath).arg(convertedFileName)) )
	copy.remove(); //file already exists
    }

  //write template in sg file
  QFile file(filepath);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
      QTextStream stream (&file);
      stream << songTemplate();
      file.close();
    }
  else
    {
      qWarning() << " CMainWindow::newsong unable to open file " << filepath << " in write mode ";
    }

  //add the song to the library
  parent()->refreshLibrary();
  close();
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
//------------------------------------------------------------------------------
QString CDialogNewSong::album() const
{
  return m_album;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setAlbum(QString AAlbum )
{
  m_album = AAlbum;
}
//------------------------------------------------------------------------------
QString CDialogNewSong::cover() const
{
  return m_cover;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setCover(QString ACover )
{
  m_cover = ACover;
}
//------------------------------------------------------------------------------
QString CDialogNewSong::lang() const
{
  return m_lang;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setLang(const QString & ALang )
{
  m_lang = ALang;
}
//------------------------------------------------------------------------------
void CDialogNewSong::browseCover()
{
  QString directory = QFileDialog::getOpenFileName(this, tr("Select cover image"),
						   QDir::home().path(),
						   tr("Images (*.jpg)"));

  if (!directory.isEmpty())
      m_coverEdit->setText(directory);
}
//------------------------------------------------------------------------------
QString CDialogNewSong::workingPath() const
{
  return m_workingPath;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setWorkingPath(QString value)
{
  m_workingPath = value;
}
//------------------------------------------------------------------------------
CMainWindow* CDialogNewSong::parent() const
{
  return m_parent ;
}
