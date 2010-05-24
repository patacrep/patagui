// Copyright (C) 2009 Romain Goffe, Alexandre Dupas
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
#ifndef __CONFIGDIALOG_H__
#define __CONFIGDIALOG_H__

#include <QDialog>
#include <QWidget>
#include <QProcess>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;
class QLineEdit;
class QCheckBox;
class QRadioButton;
class QLabel;
class QSlider;

class ConfigDialog : public QDialog
{
  Q_OBJECT
  
public:
  ConfigDialog();

  QSize sizeHint() const;

public slots:
  void changePage(QListWidgetItem *current, QListWidgetItem *previous);

protected:
  void closeEvent(QCloseEvent *event);

private:
  void createIcons();

  QListWidget *contentsWidget;
  QStackedWidget *pagesWidget;
};


class DisplayPage : public QWidget
{
  Q_OBJECT

public:
  DisplayPage(QWidget *parent = 0);

protected:
  void closeEvent(QCloseEvent *event);

private:
  void readSettings();
  void writeSettings();

  QCheckBox *artistCheckBox;
  QCheckBox *titleCheckBox;
  QCheckBox *pathCheckBox;
  QCheckBox *albumCheckBox;
  QCheckBox *lilypondCheckBox;
  QCheckBox *coverCheckBox;
};

class OptionsPage : public QWidget
{
  Q_OBJECT

public:
  OptionsPage(QWidget *parent = 0);

protected:
  void closeEvent(QCloseEvent *event);

private slots:
  void browse();
  void browseHeaderPicture();
  void pickColor();
  void resetColor();
  void checkWorkingPath(const QString&);
  void checkLilypondVersion(int);
  void processError(QProcess::ProcessError error);
  void readProcessOut();

private:
  void readSettings();
  void writeSettings();
  void updateHeader();
  void updateCustom();

  QLineEdit * workingPath;
  QLabel* m_workingPathValid;

  QRadioButton *chordbookRadioButton;
  QRadioButton *lyricbookRadioButton;

  QCheckBox *diagramCheckBox;
  QCheckBox *lilypondCheckBox;
  QCheckBox *tablatureCheckBox;

  //check lilypond version
  QLabel* m_lilypondLabel;
  QProcess* m_lilypondCheck;
  QProcess* m_grep;
  
  //Header
  QLineEdit* m_title;
  QLineEdit* m_subtitle;
  QLineEdit* m_author;
  QLineEdit* m_version;
  QLineEdit* m_mail;
  QLineEdit* m_picture;
  QLineEdit* m_copyright;

  //Custom
  QColor* m_color;
  QLabel* m_colorLabel;
  QSlider* m_sliderFontSize;
  
  bool isValid;
};

#endif // __CONFIGDIALOG_H__
