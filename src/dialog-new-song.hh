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

class CMainWindow;

class CDialogNewSong : public QDialog
{
  Q_OBJECT

public:
  CDialogNewSong(CMainWindow* AParent);
  virtual ~CDialogNewSong();

private slots:
  void setTitle(const QString &title);
  void setArtist(const QString &artist);
  void setNbColumns(int);
  void setCapo(int);
  void setAlbum(const QString &album);
  void setCover(const QString &cover);
  void setLang(const QString &lang);
  void setWorkingPath(const QString &path);
  void addSong();
  void accept();

private:
  CMainWindow* parent() const;
  QString title() const;
  QString artist() const;
  int nbColumns() const;
  int capo() const;
  QString album() const;
  QString cover() const;
  QString lang() const;
  QString workingPath() const;
  bool checkRequiredFields();
  QString songTemplate();

  CMainWindow* m_parent;
  QString m_workingPath;
  QString m_title;
  QString m_artist;
  QString m_album;
  QString m_cover;
  QString m_lang;
  int m_nbColumns;
  int m_capo;

  QLineEdit* m_titleEdit;
  QLineEdit* m_artistEdit;
};

#endif // __DIALOG_NEW_SONG_HH__
