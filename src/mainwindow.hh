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

#include <QtGui>

class CSongbook;
class CLibrary;
class CTabWidget;
class CDialogNewSong;
class CSongEditor;
class CLabel;
class CTabWidget;
class CFilterLineEdit;

/** \class CMainWindow "mainWindow.hh"
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
  QTableView * view() const;
  CLibrary * library() const;
  CSongbook * songbook() const;
  const QString workingPath();

public slots:
  void setWorkingPath(const QString &path);
  void templateSettings();
  void updateSongbookLabels();
  void updateView();

signals:
  void workingPathChanged(const QString &path);

protected:
  void closeEvent(QCloseEvent *event);

private slots:
  //songbook
  void newSongbook();
  void open();
  void save(bool forced=false);
  void saveAs();
  void build();
  void closeTab(int index);
  void changeTab(int index);

  //library
  void newSong();
  void songEditor();
  void deleteSong();

  void songEditor(const QString &filename, const QString &title = QString());
  void deleteSong(const QString &filename);

  //model
  void selectAll();
  void unselectAll();
  void invertSelection();
  void selectLanguage(bool);
  void updateSongsList();
  void filterChanged(const QString &filter);
  void selectionChanged();
  void selectionChanged(const QItemSelection &selected , const QItemSelection & deselected );

  void connectDatabase();
  void disconnectDatabase();

  //application
  void preferences();
  void applySettings();
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

  QGridLayout * songbookInfo();

  QStringList getSelectedSongs();

  bool isToolBarDisplayed();
  bool isStatusbarDisplayed();

  QItemSelectionModel * selectionModel();

  // Song library and view
  CLibrary *m_library;
  QSortFilterProxyModel *m_proxyModel;

  // Songbook widget
  CSongbook *m_songbook;
  uint m_sbNbSelected;
  uint m_sbNbTotal;
  CLabel* m_sbInfoSelection;
  CLabel* m_sbInfoTitle;
  CLabel* m_sbInfoAuthors;
  CLabel* m_sbInfoStyle;

  // Widgets
  CTabWidget* m_mainWidget;
  QTableView *m_view;
  QProgressBar* m_progressBar;
  QTextEdit* m_noDataInfo;
  CFilterLineEdit *m_filterLineEdit;

  // Global
  QString m_workingPath;

  bool m_displayColumnArtist;
  bool m_displayColumnTitle;
  bool m_displayColumnPath;
  bool m_displayColumnAlbum;
  bool m_displayColumnLilypond;
  bool m_displayColumnCover;
  bool m_displayColumnLang;
  bool m_displayCompilationLog;

  bool m_isToolBarDisplayed;
  bool m_isStatusbarDisplayed;

  CDialogNewSong *m_newSongDialog;

  //Logs
  QTextEdit* m_log;

  // Menus
  QMenu *m_fileMenu;
  QMenu *m_editMenu;
  QMenu *m_dbMenu;
  QMenu *m_viewMenu;
  QMenu *m_helpMenu;

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
