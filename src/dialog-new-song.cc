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
#include "library.hh"
#include "file-chooser.hh"
#include <QLayout>

CDialogNewSong::CDialogNewSong(CMainWindow* AParent)
  : QDialog()
  ,m_parent(AParent)
  ,m_workingPath()
  ,m_title()
  ,m_artist()
  ,m_nbColumns(2)
  ,m_capo(0)
  , m_path()
  ,m_titleEdit(new QLineEdit)
  ,m_artistEdit(new QLineEdit)
{
  m_workingPath = parent()->workingPath();
  connect(parent(), SIGNAL(workingPathChanged(QString)),
	  this, SLOT(setWorkingPath(QString)));

  //Optional fields
  QLineEdit* albumEdit = new QLineEdit;

  CFileChooser* coverEdit = new CFileChooser();
  coverEdit->setType(CFileChooser::OpenFileChooser);
  coverEdit->setCaption(tr("Select cover image"));
  coverEdit->setFilter(tr("Images (*.jpg)"));

  QComboBox* langComboBox = new QComboBox;
  langComboBox->addItem ("english");
  langComboBox->addItem ("french");
  langComboBox->addItem ("spanish");
  setLang(langComboBox->currentText());

  QSpinBox* nbColumnsEdit = new QSpinBox;
  nbColumnsEdit->setValue(m_nbColumns);
  nbColumnsEdit->setRange(0,10);

  QSpinBox* capoEdit      = new QSpinBox;
  capoEdit->setRange(0,20);

  // Action buttons
  QDialogButtonBox * buttonBox =
    new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

  connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
  connect(buttonBox, SIGNAL(rejected()), this, SLOT(close()));

  QGroupBox* requiredFieldsBox = new QGroupBox(tr("Required fields"));
  QFormLayout *requiredLayout = new QFormLayout;
  requiredLayout->addRow(tr("&Title:"), m_titleEdit);
  requiredLayout->addRow(tr("&Artist:"), m_artistEdit);
  requiredFieldsBox->setLayout(requiredLayout);

  QGroupBox* optionalFieldsBox = new QGroupBox(tr("Optional fields"));
  QFormLayout* optionalLayout = new QFormLayout;
  optionalLayout->addRow(tr("A&lbum:"),     albumEdit);
  optionalLayout->addRow(tr("&Cover:"),     coverEdit);
  optionalLayout->addRow(tr("&Language: "), langComboBox);
  optionalLayout->addRow(tr("C&olumns: "),  nbColumnsEdit);
  optionalLayout->addRow(tr("Ca&po: "),     capoEdit);
  optionalFieldsBox->setLayout(optionalLayout);

  QBoxLayout* layout = new QVBoxLayout;
  layout->addWidget(requiredFieldsBox);
  layout->addWidget(optionalFieldsBox);
  layout->addWidget(buttonBox);

  connect(m_titleEdit,  SIGNAL(textChanged(QString)), this, SLOT(setTitle(QString)) );
  connect(m_artistEdit, SIGNAL(textChanged(QString)), this, SLOT(setArtist(QString)) );

  connect(langComboBox, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(setLang(const QString&)) );
  connect(nbColumnsEdit, SIGNAL(valueChanged(int)), this, SLOT(setNbColumns(int)) );
  connect(capoEdit, SIGNAL(valueChanged(int)), this, SLOT(setCapo(int)) );
  connect(albumEdit, SIGNAL(textChanged(QString)), this, SLOT(setAlbum(QString)) );
  connect(coverEdit, SIGNAL(pathChanged(const QString&)), this, SLOT(setCover(const QString&)) );

  setLayout(layout);
  setWindowTitle(tr("New song"));
  show();
}
//------------------------------------------------------------------------------
CDialogNewSong::~CDialogNewSong()
{}
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
  parent()->library()->addSong(filepath);

  m_path = filepath;

  QDialog::accept();
}
//------------------------------------------------------------------------------
QString CDialogNewSong::title() const
{
  return m_title;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setTitle(const QString &ATitle)
{
  m_title = ATitle;
}
//------------------------------------------------------------------------------
QString CDialogNewSong::artist() const
{
  return m_artist;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setArtist(const QString &AArtist)
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
void CDialogNewSong::setAlbum(const QString &AAlbum)
{
  m_album = AAlbum;
}
//------------------------------------------------------------------------------
QString CDialogNewSong::cover() const
{
  return m_cover;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setCover(const QString &ACover)
{
  m_cover = ACover;
}
//------------------------------------------------------------------------------
QString CDialogNewSong::lang() const
{
  return m_lang;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setLang(const QString & ALang)
{
  m_lang = ALang;
}
//------------------------------------------------------------------------------
QString CDialogNewSong::workingPath() const
{
  return m_workingPath;
}
//------------------------------------------------------------------------------
void CDialogNewSong::setWorkingPath(const QString &path)
{
  m_workingPath = path;
}
//------------------------------------------------------------------------------
CMainWindow* CDialogNewSong::parent() const
{
  return m_parent ;
}
//------------------------------------------------------------------------------
QString CDialogNewSong::path() const
{
  return m_path;
}
//------------------------------------------------------------------------------
