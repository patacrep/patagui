// Copyright (C) 2009-2011, Romain Goffe <romain.goffe@gmail.com>
// Copyright (C) 2009-2011, Alexandre Dupas <alexandre.dupas@gmail.com>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// 02110-1301, USA.
//******************************************************************************
#ifndef __PREFERENCES_HH__
#define __PREFERENCES_HH__

#include <QDialog>
#include <QWidget>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;

class QLabel;
class QLineEdit;
class QCheckBox;
class QSpinBox;
class CFileChooser;

/** \brief ConfigDialog handles the display of the config pages
 */
class ConfigDialog : public QDialog
{
  Q_OBJECT

public:
  ConfigDialog();

public slots:
  void changePage(QListWidgetItem *current, QListWidgetItem *previous);

protected:
  void closeEvent(QCloseEvent *event);

private:
  void createIcons();

  QListWidget *m_contentsWidget;
  QStackedWidget *m_pagesWidget;
};

/** \brief DisplayPage is the config page used to specify display options
 */
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
  QCheckBox *m_langCheckBox;
  QCheckBox *m_compilationLogCheckBox;
};

/** \brief OptionPage is the config page used to specify general options
 */
class OptionsPage : public QWidget
{
  Q_OBJECT

public:
  OptionsPage(QWidget *parent = 0);

protected:
  void closeEvent(QCloseEvent *event);

private slots:
  void checkWorkingPath(const QString &path);

private:
  void readSettings();
  void writeSettings();

  CFileChooser *m_workingPath;
  QLabel *m_workingPathValid;
};

/** \brief NetworkPage is the config page used to specify network options
 */
class NetworkPage : public QWidget
{
  Q_OBJECT

public:
  NetworkPage(QWidget *parent = 0);

protected:
  void closeEvent(QCloseEvent *event);

private:
  void readSettings();
  void writeSettings();

  QLineEdit *m_hostname;
  QSpinBox *m_port;
  QLineEdit *m_user;
  QLineEdit *m_password;
};

#endif // __PREFERENCES_HH__
