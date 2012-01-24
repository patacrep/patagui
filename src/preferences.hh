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
#include <QScrollArea>

#if defined(Q_OS_WIN32)
#define PLATFORM_BUILD_COMMAND "cmd.exe /C windows\\make.bat %basename"
#define PLATFORM_CLEAN_COMMAND "cmd.exe /C windows\\clean.bat"
#define PLATFORM_CLEANALL_COMMAND "cmd.exe /C windows\\cleanall.bat"
#elif defined(Q_OS_MAC)
#define PLATFORM_BUILD_COMMAND "macos/make.sh %basename"
#define PLATFORM_CLEAN_COMMAND "macos/clean.sh"
#define PLATFORM_CLEANALL_COMMAND "macos/cleanall.sh"
#else //Unix/Linux
#define PLATFORM_BUILD_COMMAND "make %target"
#define PLATFORM_CLEAN_COMMAND "make clean"
#define PLATFORM_CLEANALL_COMMAND "make cleanall"
#endif

#include "config.hh"

class QListWidget;
class QListWidgetItem;
class QStackedWidget;

class QLabel;
class QLineEdit;
class QCheckBox;
class QSpinBox;
class CFileChooser;
class CMainWindow;

class QtGroupBoxPropertyBrowser;

/** \brief ConfigDialog handles the display of the config pages
 */
class ConfigDialog : public QDialog
{
  Q_OBJECT

public:
  ConfigDialog(CMainWindow* parent);
  CMainWindow* parent() const;				   

public slots:
  void changePage(QListWidgetItem *current, QListWidgetItem *previous);

protected:
  void closeEvent(QCloseEvent *event);

private:
  void createIcons();

  CMainWindow *m_parent;
  QListWidget *m_contentsWidget;
  QStackedWidget *m_pagesWidget;
};

/** \brief Page is the base class for config page
 */
class Page : public QScrollArea
{
  Q_OBJECT
public:
  Page(ConfigDialog *configDialog);

  ConfigDialog * configDialog() const;

  void setLayout(QLayout *layout);

protected:
  void closeEvent(QCloseEvent *event);

private:
  virtual void readSettings();
  virtual void writeSettings();

  QWidget *m_content;
  ConfigDialog *m_configDialog;
};

/** \brief DisplayPage is the config page used to specify display options
 */
class DisplayPage : public Page
{
  Q_OBJECT

public:
  DisplayPage(ConfigDialog *configDialog);

private:
  void readSettings();
  void writeSettings();

  QCheckBox *m_titleCheckBox;
  QCheckBox *m_artistCheckBox;
  QCheckBox *m_pathCheckBox;
  QCheckBox *m_albumCheckBox;
  QCheckBox *m_lilypondCheckBox;
  QCheckBox *m_langCheckBox;
  QCheckBox *m_compilationLogCheckBox;
};

/** \brief OptionsPage is the config page used to specify general options
 */
class OptionsPage : public Page
{
  Q_OBJECT

public:
  OptionsPage(ConfigDialog *configDialog);

private slots:
  void checkWorkingPath(const QString &path);
  void resetBuildCommand();
  void resetCleanCommand();
  void resetCleanallCommand();

private:
  void readSettings();
  void writeSettings();

  CFileChooser *m_workingPath;
  QLabel *m_workingPathValid;

  QLineEdit *m_buildCommand;
  QLineEdit *m_cleanCommand;
  QLineEdit *m_cleanallCommand;
};

/** \brief Editor is the config page used to specify options related
    to the song editor
 */
class EditorPage : public Page
{
  Q_OBJECT

public:
  EditorPage(ConfigDialog *configDialog);

private slots:
  void selectFont();
  void updateFontButton();

private:
  void readSettings();
  void writeSettings();

  QCheckBox *m_numberLinesCheckBox;
  QCheckBox *m_highlightCurrentLineCheckBox;
  QPushButton *m_fontButton;
  QFont m_font;
  QString m_fontstr;
};

#ifdef ENABLE_LIBRARY_DOWNLOAD

/** \brief NetworkPage is the config page used to specify network options
 */
class NetworkPage : public Page
{
  Q_OBJECT

public:
  NetworkPage(ConfigDialog *configDialog);

private:
  void readSettings();
  void writeSettings();

  QLineEdit *m_hostname;
  QSpinBox *m_port;
  QLineEdit *m_user;
  QLineEdit *m_password;
};

#endif // ENABLE_LIBRARY_DOWNLOAD

/** \brief SongbookPage is the config page used to specify general songbooks
 */
class SongbookPage : public Page
{
  Q_OBJECT

public:
  SongbookPage(ConfigDialog *configDialog);
  
private slots:
  void updatePropertyEditor();

private:
  QtGroupBoxPropertyBrowser * m_propertyEditor;
};

#endif // __PREFERENCES_HH__
