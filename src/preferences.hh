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

  QCheckBox *m_artistCheckBox;
  QCheckBox *m_titleCheckBox;
  QCheckBox *m_pathCheckBox;
  QCheckBox *m_albumCheckBox;
  QCheckBox *m_lilypondCheckBox;
  QCheckBox *m_coverCheckBox;
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
  void checkWorkingPath(const QString&);
  void checkLilypondVersion(int);
  void processError(QProcess::ProcessError error);
  void lyricBookMode(bool);

private:
  void readSettings();
  void writeSettings();

  QLineEdit * m_workingPath;
  QLabel* m_workingPathValid;

  //check lilypond version
  QLabel* m_lilypondLabel;
  QProcess* m_lilypondCheck;

  bool m_isValid;
};

#endif // __CONFIGDIALOG_H__
