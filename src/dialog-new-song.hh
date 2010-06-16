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
#ifndef __DIALOG_NEW_SONG_HH__
#define __DIALOG_NEW_SONG_HH__

#include <QtGui>

class CDialogNewSong : public QDialog
{
  Q_OBJECT
  
public:
  CDialogNewSong();

public slots:
  QString title() const;
  void setTitle(QString);

  QString artist() const;
  void setArtist(QString);

  int nbColumns() const;
  void setNbColumns(int);

  int capo() const;
  void setCapo(int);

  QString album() const;
  void setAlbum(QString);

  QString cover() const;
  void setCover(QString);

private slots:
  void browseCover();

private:
  //required fields
  QString m_title;
  QString m_artist;

  //optional fields
  QString m_album;
  QString m_cover;
  QLineEdit* m_coverEdit;
  uint m_nbColumns;
  uint m_capo;
};

#endif // __DIALOG_NEW_SONG_HH__
