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
#ifndef __SONGBOOK_HH__
#define __SONGBOOK_HH__

#include <QObject>
#include <QString>
#include <QStringList>

class QWidget;
class QLabel;
class QLineEdit;
class QSlider;

class CSongbook : public QObject
{
  Q_OBJECT

public:
  CSongbook();
  ~CSongbook();

  QString title();
  void setTitle(const QString &title);

  QString subtitle();
  void setSubtitle(const QString &subtitle);

  QString author();
  void setAuthor(const QString &author);

  QString version();
  void setVersion(const QString &version);

  QString mail();
  void setMail(const QString &mail);  

  QString picture();
  void setPicture(const QString &picture);

  QString pictureCopyright();
  void setPictureCopyright(const QString &pictureCopyright);

  QString shadeColor();
  void setShadeColor(const QString &shadeColor);

  QString fontSize();
  void setFontSize(QString &fontSize);

  QStringList songs();
  void setSongs(QStringList songs);

  void save(QString &filename);
  void load(QString &filename);

  QWidget *panel();

private slots:
  void pickShadeColor();
  void browsePicture();

private:
  QString m_title;
  QString m_subtitle;
  QString m_author;
  QString m_version;
  QString m_mail;
  QString m_picture;
  QString m_pictureCopyright;
  QString m_shadeColor;
  QString m_fontSize;

  QStringList m_songs;

  // panel widgets
  QWidget *m_panel;
  QLineEdit *m_titleEdit;
  QLineEdit *m_subtitleEdit;
  QLineEdit *m_authorEdit;
  QLineEdit *m_versionEdit;
  QLineEdit *m_mailEdit;
  QLineEdit *m_pictureEdit;
  QLineEdit *m_pictureCopyrightEdit;
  QLabel *m_shadeColorLabel;
  QSlider *m_fontSizeSlider;
};

#endif // __SONGBOOK_HH__
