// Copyright (C) 2010 Romain Goffe, Alexandre Dupas
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
#ifndef SONG_EDITOR_HH
#define SONG_EDITOR_HH

#include <QWidget>
#include <QString>

class QTextEdit;

class CSongEditor : public QWidget
{
  Q_OBJECT

public:
  CSongEditor(const QString & APath);
  virtual ~CSongEditor();
  
  QString filePath();
  void setFilePath(const QString & APath);

private slots:
  void save();

private:

  QTextEdit* m_textEdit;
  QString m_filePath;
};
#endif //SONG_EDITOR_HH
