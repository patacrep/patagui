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

/**
 * \file mainWindow.hh
 *
 * Class for the main window of the application.
 *
 */

#ifndef __MAIN_WINDOW_HH__
#define __MAIN_WINDOW_HH__

#include <QMainWindow>

#include <QModelIndex>
#include <QDir>
#include <QDate>
#include <QLocale>

class CSongbook;
class CLibrary;
class CLibraryView;
class CTabWidget;
class CDialogNewSong;
class CSongEditor;
class CLabel;
class CTabWidget;
class CFilterLineEdit;
class CNotification;

class QProgressBar;
class QTextEdit;
class QItemSelectionModel;
class QSortFilterProxyModel;
class QLabel;

/** \class CMainWindow "main-window.hh"
 * \brief CMainWindow is the base class of the application
 */
class CMainWindow : public QMainWindow
{
  Q_OBJECT

public:
  CMainWindow();
  ~CMainWindow();

  QProgressBar * progressBar() const;
  QTextEdit * log() const;
  CLibraryView * view() const;
  CLibrary * library() const;
  CSongbook * songbook() const;
  const QString workingPath();

protected:
  void closeEvent(QCloseEvent *event);

private slots:
  //songbook
  void newSongbook();
  void open();
  void save(bool forced = false);
  void saveAs();
  void build();
  void closeTab(int index);
  void changeTab(int index);

  //library
  void newSong();
  void songEditor(const QModelIndex &index = QModelIndex());
  void deleteSong();

  void songEditor(const QString &filename, const QString &title = QString());
  void deleteSong(const QString &filename);
  void updateNotification(const QString &path);
  void noDataNotification(const QDir &directory);

  //model
  void selectLanguage();
  void filterChanged(const QString &filter);
  void selectedSongsChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);

  //application
  void preferences();
  void setToolBarDisplayed(bool);
  void setStatusbarDisplayed(bool);
  void documentation();
  void about();

  void updateTitle(const QString &filename);
  void switchToolBar(QToolBar *toolBar);

private:
  void readSettings();
  void writeSettings();

  void createActions();
  void createMenus();
  void createToolBar();

  bool isToolBarDisplayed();
  bool isStatusbarDisplayed();

  QItemSelectionModel * selectionModel();

  // Models and views
  CLibrary *m_library;
  CLibraryView *m_view;
  CSongbook *m_songbook;
  QSortFilterProxyModel *m_proxyModel;

  // Widgets
  CTabWidget *m_mainWidget;
  QProgressBar *m_progressBar;
  CNotification *m_noDataInfo;
  CNotification *m_updateAvailable;
  QLabel *m_infoSelection;
  CFilterLineEdit *m_filterLineEdit;
  QTextEdit *m_log;

  // Settings
  QString m_workingPath;
  bool m_isToolBarDisplayed;
  bool m_isStatusbarDisplayed;

  CDialogNewSong *m_newSongDialog;

  // Menus
  QMenu *m_editorMenu;

  QToolBar *m_toolBar;
  QToolBar *m_currentToolBar;

  // Application actions
  QAction *m_preferencesAct;
  QAction *m_toolBarViewAct;
  QAction *m_statusbarViewAct;
  QAction *m_adjustColumnsAct;
  QAction *m_documentationAct;
  QAction *m_aboutAct;
  QAction *m_exitAct;

  // Songbook actions
  QAction *m_newAct;
  QAction *m_openAct;
  QAction *m_saveAct;
  QAction *m_saveAsAct;
  QAction *m_buildAct;
  QAction *m_cleanAct;
  QAction *m_sbInfoAct;

  // Library action
  QAction *m_newSongAct;
  QAction *m_selectAllAct;
  QAction *m_unselectAllAct;
  QAction *m_invertSelectionAct;
  QAction *m_selectEnglishAct;
  QAction *m_selectFrenchAct;
  QAction *m_selectSpanishAct;
  QAction *m_libraryUpdateAct;
  QAction *m_libraryDownloadAct;

  // Editors
  QMap< QString, CSongEditor* > m_editors;
};

#endif  // __MAIN_WINDOW_HH__
