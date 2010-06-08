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
  Q_PROPERTY(QString title READ title WRITE setTitle)
  Q_PROPERTY(QString author READ author WRITE setAuthor)
  Q_PROPERTY(QString subtitle READ subtitle WRITE setSubtitle)
  Q_PROPERTY(QString version READ version WRITE setVersion)
  Q_PROPERTY(QString mail READ mail WRITE setMail)
  Q_PROPERTY(QString picture READ picture WRITE setPicture)
  Q_PROPERTY(QString pictureCopyright READ pictureCopyright WRITE setPictureCopyright)
  Q_PROPERTY(QString footer READ footer WRITE setFooter)
  Q_PROPERTY(QString licence READ licence WRITE setLicence)
  Q_PROPERTY(QString shadeColor READ shadeColor WRITE setShadeColor)
  Q_PROPERTY(QString fontSize READ fontSize WRITE setFontSize)
  Q_PROPERTY(QString tmpl READ tmpl WRITE setTmpl)
  Q_PROPERTY(QStringList bookType READ bookType WRITE setBookType)
  Q_PROPERTY(QStringList songs READ songs WRITE setSongs)

public slots:
  void setTitle(const QString &title);
  void setSubtitle(const QString &subtitle);
  void setAuthor(const QString &author);
  void setVersion(const QString &version);
  void setMail(const QString &mail);  
  void setPicture(const QString &picture);
  void setPictureCopyright(const QString &pictureCopyright);
  void setShadeColor(const QString &shadeColor);
  void setFontSize(const QString &fontSize);
  void setFooter(const QString &footer);
  void setLicence(const QString &licence);
  void setTmpl(const QString &tmpl);
  void setBookType(QStringList bookType);
  void setSongs(QStringList songs);

  void save(const QString &filename);
  void load(const QString &filename);

public:
  CSongbook();
  ~CSongbook();

  QString title();
  QString subtitle();
  QString author();
  QString version();
  QString mail();
  QString picture();
  QString pictureCopyright();
  QString shadeColor();
  QString fontSize();
  QString footer();
  QString licence();
  QString tmpl();

  QStringList bookType();
  QStringList songs();

  QWidget *panel();

private slots:
  void pickShadeColor();
  void browsePicture();

  void update();

private:
  QString m_title;
  QString m_subtitle;
  QString m_author;
  QString m_version;
  QString m_mail;
  QString m_picture;
  QString m_pictureCopyright;
  QString m_footer;
  QString m_licence;
  QString m_shadeColor;
  QString m_fontSize;
  QString m_tmpl;

  QStringList m_bookType;
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
  QLineEdit *m_footerEdit;
  QLineEdit *m_licenceEdit;
  QLabel *m_shadeColorLabel;
  QSlider *m_fontSizeSlider;
};

#endif // __SONGBOOK_HH__
